// Stitching.cpp : �������̨Ӧ�ó������ڵ㡣

#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include <qprogressdialog.h>
#include "ui_stitchingvsqt.h"
#include <iostream>
#include <fstream>
#include <string>
#include <tchar.h>
#include <Windows.h>
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"
#include "stitching.h"
#include "qmessagebox.h"
#include <QRunnable>
#include "stitchingvsqt.h"


using namespace std;
using namespace cv;
using namespace cv::detail;

typedef void*(*STITCH)(std::vector<std::vector<std::string>> &img_names,
	const void *config,
	std::string &msg);

typedef void (*PANOIMAGE)(void *img, const std::string &name);

/*
�������̣�
1.�����е��ó�������Դͼ���Լ�����Ĳ���

2.�������⣬�ж���ʹ��surf����orb��Ĭ����surf��

3.��ͼ������������ƥ�䣬ʹ������ںʹν��ڷ��������������ŵ�ƥ������Ŷȱ���������

4.��ͼ����������Լ������Ŷȸߵ�ͼ�񱣴浽ͬһ�������У�ɾ�����ŶȱȽϵ͵�ͼ����ƥ�䣬�õ�����ȷƥ���ͼ�����С����������Ŷȸ������޵�����ƥ��ϲ���һ�������С�

5.������ͼ���������������Թ��ƣ�Ȼ�������ת����

6.ʹ�ù���ƽ������һ����׼�Ĺ��Ƴ���ת����

7.����У����ˮƽ���ߴ�ֱ

8.ƴ��

9.�ںϣ���Ƶ���ںϣ����ղ���
*/

bool preview = false; //��Ԥ��ģʽ���г��򣬱�����ģʽҪ�죬�����ͼ��ֱ��ʵͣ�ƴ�ӵķֱ���compose_megapix ����Ϊ0.6
bool try_gpu = false;
double work_megapix = 0.6;//ͼ��ƥ��ķֱ��ʴ�С��ͼ�������ߴ��Ϊwork_megapix*100000��Ĭ��Ϊ0.6
double seam_megapix = 0.1; //ƴ�ӷ����صĴ�С Ĭ����0.1
double compose_megapix = -1; //ƴ�ӷֱ���
float conf_thresh = 1.f; //����ͼ����ͬһȫ��ͼ�����Ŷȣ�Ĭ��Ϊ1.0
string features_type = "surf";
//string features_type = "orb";
string ba_cost_func = "ray";   //����ƽ����������ѡ��
//string ba_cost_func = "reproj";

string ba_refine_mask = "xxxxx";
bool do_wave_correct = true;
WaveCorrectKind wave_correct = detail::WAVE_CORRECT_HORIZ;//����У�� ˮƽ����ֱ����û�� Ĭ����horiz
bool save_graph = false;
std::string save_graph_to;
string warp_type = "cylindrical";
int expos_comp_type = ExposureCompensator::GAIN_BLOCKS;
float match_conf = 0.65f;//�����������ŵȼ��������ƥ�������ν���ƥ�����ı�ֵ��surfĬ��Ϊ0.65��orbĬ��Ϊ0.3
string seam_find_type = "gc_color"; // ƴ�ӷ�϶���Ʒ���
int blend_type = Blender::MULTI_BAND;// �ںϷ�����Ĭ���Ƕ�Ƶ���ں�
float blend_strength = 5;//�ں�ǿ�ȣ�0-100
string result_name = "result.jpg";

vector<std::vector<std::string>> imageNames;
vector<int> idx;

class StitchingVSQt;

string& replace_all_distinct(string&   str, const   string&   old_value, const   string&   new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length())   {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

int startCalculation()
{
	
	StitchingVSQt s;
	std::string msg;
	const char* dllName = "stitching.dll";
    const char* funName1 = "stitch";
	const char* funName2 = "pano_image";
	
	HINSTANCE hDll = LoadLibrary(TEXT("stitching.dll"));
	if (hDll != NULL){
		STITCH fp = STITCH(GetProcAddress(hDll, "stitch"));
		PANOIMAGE fPa = PANOIMAGE(GetProcAddress(hDll, "writeImage"));
		if (fp != NULL){
			//s.showMessage(QString::fromLocal8Bit("�ҵ�dll�еĺ���"));
			void* p=fp(imageNames, nullptr, msg);
			//s.showMessage(QString::fromLocal8Bit("��һ�������������"));

			QString fileName(QString::fromStdString(result_name));
			QString dir = QFileDialog::getExistingDirectory(NULL, QString::fromLocal8Bit("ѡ������ͼƬ����Ŀ¼"),
				"/home",
				QFileDialog::ShowDirsOnly
				| QFileDialog::DontResolveSymlinks);
			s.changePicture(dir + "/" + fileName);

			fPa(p, (dir + "/" + fileName).toStdString());
			//s.showMessage(QString::fromLocal8Bit("�ڶ��������������"));
			/*img_names.clear();*/
			imageNames.clear();
			idx.clear();
			delete p;
		}
		else{
			s.showMessage(QString::fromLocal8Bit("δ�ҵ�dll�еĺ���"));
		}
		FreeLibrary(hDll);
	}
	else {
		
		s.showMessage(QString::fromLocal8Bit("δ�ҵ�dll"));
		return -1;
	}
	return 0;
}


QProgressDialog progress(QString::fromLocal8Bit("���ڵ���ͼƬ���ݣ����Ժ�..."),
		QString::fromLocal8Bit("ȡ��"),
		0, 12, // Range
		NULL);

void controlProgress(int status){
	switch (status)
	{
	case 1:
		progress.show();
		qApp->processEvents();
		progress.setWindowModality(Qt::WindowModal);
		progress.setWindowTitle(QString::fromLocal8Bit("���ڵ���ͼƬ���ݣ����Ժ�..."));

		progress.setValue(1);
		progress.setModal(true);
		progress.setLabelText(QString::fromLocal8Bit("��ȡͼƬ�����Ժ�..."));
		break;
	case 2:
		progress.setValue(2);
		progress.setLabelText(QString::fromLocal8Bit("��������ȡ...."));
		break;
	case 3:
		progress.setValue(3);
		progress.setLabelText(QString::fromLocal8Bit("������ƥ��...."));
		break;
	case 4:
		progress.setValue(4);
		progress.setLabelText(QString::fromLocal8Bit("�����������������任����...."));
		break;
	case 5:
		progress.setValue(5);
		progress.setLabelText(QString::fromLocal8Bit("���ν���...."));
		break;
	case 6:
		progress.setValue(6);
		progress.setLabelText(QString::fromLocal8Bit("����ͶӰ...."));
		break;
	case 7:
		progress.setValue(7);
		break;
	case 8:
		progress.setValue(8);
		break;
	case 9:
		progress.setValue(9);
		progress.setLabelText(QString::fromLocal8Bit("ͼ���ں�...."));
		break;
	case 10:
		progress.setValue(10);
		progress.setLabelText(QString::fromLocal8Bit("���ղ���...."));
		break;
	case 11:
		progress.setValue(11);
		progress.setLabelText(QString::fromLocal8Bit("����corners�����ͼ���Сȷ������ȫ��ͼ�ߴ�...."));
		break;
	case 12:
		progress.setValue(12);
		break;
	default:
		break;
	}		
}