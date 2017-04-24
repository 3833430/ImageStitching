#include "stitchingvsqt.h"
#include "ui_stitchingvsqt.h"
#include "stitching.h"
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>
#include <iostream>
#include <fstream>
#include <QRunnable>  
#include <QCompleter>
#include <QMainWindow>
#include <QInputDialog>

#include <QtConcurrent\QtConcurrentRun>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <qgraphicsview.h>
#include <qstringlist.h>
#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

using std::string;
using std::ifstream;
using std::ofstream;
using std::cout;
using namespace cv;
bool entered;
bool refreshed;

QGraphicsScene *scene;
QGraphicsScene *scene1;
QGraphicsView* qgv;
//Ui::StitchingVSQt *ui1;

extern string features_type;
extern string ba_cost_func;
extern string warp_type;
extern string seam_find_type;
extern string result_name;
extern float  blend_strength;
extern vector<std::vector<std::string>> imageNames;
extern vector<int> idx;

QString path="";

StitchingVSQt::StitchingVSQt(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::StitchingVSQt)
{
	ui->setupUi(this);
	QStringList strings;
	strings << "surf" << "orb" ;
	QStringList strings1;
	strings1 << "ray" << "reproj";
	QStringList strings2;
	strings2 << "spherical " << "plane" << "cylindrical";
	QStringList strings3;
	strings3 << "gc_color " << "voronoi" << "gc_colorgrad"<<"no";

	cvSetErrMode(CV_ErrModeParent);
	/*cvRedirectError(errorReport);*/

	ui->comboBox1->addItems(strings);
	ui->comboBox2->addItems(strings1);
	ui->comboBox3->addItems(strings2);
	ui->comboBox4->addItems(strings3);

	ui->frame->setVisible(false);

	ui->horizontalSlider->setMaximum(100);
	ui->horizontalSlider->setValue(5);

	setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
	setFixedSize(this->width(), this->height());

	entered = false;
	QAction *import = new QAction(QString::fromLocal8Bit("����ͼƬ�ļ�"), this);
	QAction *exitApp = new QAction(QString::fromLocal8Bit("�˳�"), this);
	QAction *setting = new QAction(QString::fromLocal8Bit("��ϸ��������"), this);
	QAction *calibrate = new QAction(QString::fromLocal8Bit("�궨"), this);

	ui->menu->addAction(import);
	ui->menu->addAction(exitApp);
	ui->menu_3->addAction(calibrate);
	ui->menu_2->addAction(setting);

	this->image = new QImage();
	
	 
	connect(import, SIGNAL(triggered()), this, SLOT(fileOpenActionSlot()));
	connect(exitApp, SIGNAL(triggered()), this, SLOT(fileCloseActionSlot()));
	//connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(startCal()));

	connect(setting, SIGNAL(triggered()), this, SLOT(setDetail()));
	connect(calibrate, SIGNAL(triggered()), this, SLOT(calibrate()));
	connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(finishSetting()));
	connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(back()));

	QString path1 = QCoreApplication::applicationDirPath();

	path1.append(QString("/imgBack.jpg"));

	if (image->load(path1))
	{
		QGraphicsScene *scene = new QGraphicsScene;
		scene->addPixmap(QPixmap::fromImage(*image).scaled(680, 450));
		ui->graphicsView->setScene(scene);
		ui->graphicsView->show();
	}
}

StitchingVSQt::~StitchingVSQt()
{
	/*delete ui;
    delete image;*/
}

int StitchingVSQt::errorReport(){
	QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("��ѡȡ��ͼƬ�غϳ̶ȹ���"));
	return -1;
}

void StitchingVSQt::startCal(){
	if (entered){
		try{	
			//QtConcurrent::run(startCalculation);
			startCalculation();
		}
		catch (cv::Exception& e){
			const char*m = e.what();
			string s(m);
			QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromStdString("��ѡȡ��ͼƬ�غϳ̶ȹ���"+s));
		}
		
		if (image->load(path))
		{
			QGraphicsScene *scene = new QGraphicsScene;
			scene->addPixmap(QPixmap::fromImage(*image)/*.scaled(1000,800)*/);
			ui->graphicsView->setScene(scene);
			ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
			ui->graphicsView->resize(681, 461);
			//ui->graphicsView->addScrollBarWidget->setVisible(false);
			ui->graphicsView->show();
		}
	}
	else
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("���ȵ�����Ӧ��Ҫƴ�ӵ�ͼƬ"));
	}
}

void StitchingVSQt::fileOpenActionSlot()
{
//	QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("������~���ļ�~�˵�"), QMessageBox::Yes | QMessageBox::No);
	
	vector<QStringList>imagNameTemp;
	int i = 0;
    while(true){
			std::stringstream stream;
			stream << i+1; 
			QStringList fileNames;
			//fileNames;
			QStringList	filesTemp=QFileDialog::getOpenFileNames(this,
				QString::fromLocal8Bit("��") +QString::fromStdString(stream.str()) + QString::fromLocal8Bit("��"),
				"",
				"IMAGE Files(*.jpg)",
				0);
			if ((filesTemp.length() == 0) && (imagNameTemp.size() != 0)){
				break;
			}

			if ((filesTemp.length() == 0) && (imagNameTemp.size()==0)){
				return;
			}
			fileNames.append(filesTemp);
			imagNameTemp.push_back(fileNames);
			idx.push_back(0);
			i++;
			//imagNameTemp.push_back()
			/*if (imagNameTemp.size() == 0)
			{
				return;
			}*/
	}
	//QMessageBox::StandardButton rb = QMessageBox::question(NULL, QString::fromLocal8Bit("ͼ�����"), QString::fromLocal8Bit("�Ƿ�ֲ㵼��ͼ��"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	//if (rb == QMessageBox::Yes)
	//{
	//	bool isOK;
	//	int level = 0;
	//	QString text = QInputDialog::getText(NULL, QString::fromLocal8Bit("����"),
	//		QString::fromLocal8Bit("��������Ч��ƴ�Ӳ���"),
	//		QLineEdit::Normal,
	//		"",
	//		&isOK);
	//	if (isOK) {
	//		level = text.toInt();
	//		if (level <= 0){
	//			return;
	//		}
	//	}
	//	else{
	//		return;
	//	}
	//	for (int i = 0; i <level; i++){
	//		std::stringstream stream;
	//		stream << i+1; 
	//		QStringList fileNames;
	//		//fileNames;
	//		QStringList	filesTemp=QFileDialog::getOpenFileNames(this,
	//			QString::fromLocal8Bit("��") +QString::fromStdString(stream.str()) + QString::fromLocal8Bit("��"),
	//			"",
	//			"IMAGE Files(*.jpg)",
	//			0);
	//		fileNames.append(filesTemp);
	//		imagNameTemp.push_back(fileNames);
	//		idx.push_back(0);
	//		//imagNameTemp.push_back()
	//		if (fileNames.length() == 0)
	//		{
	//			return;
	//		}
	//	}
	//}
	//else{
	//	QStringList fileNames;
	//	fileNames = QFileDialog::getOpenFileNames(this,
	//		tr("Open File"),
	//		"",
	//		"IMAGE Files(*.jpg)",
	//		0);
	//	if (fileNames.length() == 0)
	//	{
	//		return;
	//	}
	//	imagNameTemp.push_back(fileNames);
	//	idx.push_back(0);
	//}
	
	//QProgressDialog progress(tr("���ڵ���ͼƬ���ݣ����Ժ�..."),
	//	tr("ȡ��"),
	//	0, fileNames.length(), // Range
	//	this);
	//progress.show();
	//qApp->processEvents();
	//progress.setWindowModality(Qt::WindowModal);

	/*for (int i = 0; i < fileNames.length(); i++)
	{
		progress.setValue(i);
		string fileName = fileNames[i].toLocal8Bit().data();

		
		storeImageFileName(fileName);
		
		if (progress.wasCanceled()) {
			break;
		}
	}*/
	for (int i = 0; i < imagNameTemp.size(); i++)
	{
		vector<string> tempF;
		for (int j = 0; j < imagNameTemp[i].length(); j++){
			tempF.push_back(imagNameTemp[i][j].toLocal8Bit().data());
		}
		//progress.setValue(i);
		//string fileName = fileNames[i].toLocal8Bit().data();
		imageNames.push_back(tempF);
		//storeImageFileName(fileName);
	}
	/*progress.setValue(fileNames.length());*/
	entered = true;
//	QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("ͼƬѡȡ�ɹ���������ʼ���㰴ť"));
	startCal();
	//QtConcurrent::run(startCalculation);
}

void StitchingVSQt::fileCloseActionSlot()
{
	QApplication* app;
	app->exit(0);
}

void StitchingVSQt::showMessage(QString content){
	QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), content, QMessageBox::Yes | QMessageBox::No);
}

void StitchingVSQt::changePicture(QString path1){
	path = path1;
}

void StitchingVSQt::setDetail(){
	ui->graphicsView->setVisible(false);
//	ui->pushButton->setVisible(false);
	ui->frame->setVisible(true);
}

void StitchingVSQt::calibrate(){
	//QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("��ʼ�궨,�����ĵȴ���"));
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("�Ƿ�ʼ�궨�����ǣ������ĵȴ���"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes){
		//ifstream fin("calibdata.txt"); /* �궨����ͼ���ļ���·�� */
		QStringList fileNames = QFileDialog::getOpenFileNames(this,
			QString::fromLocal8Bit("��ѡ��궨ͼƬ"),
			"",
			"IMAGE Files(*.jpg)",
			0);
		if (fileNames.length() == 0)
		{
			return;
		}
		ofstream fout("caliberation_result.txt");  /* ����궨������ļ� */
		//��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��	
		//cout << "��ʼ��ȡ�ǵ㡭����������";
		int image_count = 0;  /* ͼ������ */
		Size image_size;  /* ͼ��ĳߴ� */
		Size board_size = Size(6, 9);    /* �궨����ÿ�С��еĽǵ��� */
		vector<Point2f> image_points_buf;  /* ����ÿ��ͼ���ϼ�⵽�Ľǵ� */
		vector<vector<Point2f>> image_points_seq; /* �����⵽�����нǵ� */
		//string filename;
		int count = -1;//���ڴ洢�ǵ������

		for (int i = 0; i < fileNames.length(); i++)
		{
			string fileName = fileNames[i].toLocal8Bit().data();
			image_count++;
			// ���ڹ۲�������
			//cout << "image_count = " << image_count << std::endl;
			///* �������*/
			//cout << "-->count = " << count;
			Mat imageInput = imread(fileName);
			if (image_count == 1)  //�����һ��ͼƬʱ��ȡͼ������Ϣ
			{
				image_size.width = imageInput.cols;
				image_size.height = imageInput.rows;
				cout << "image_size.width = " << image_size.width << std::endl;
				cout << "image_size.height = " << image_size.height << std::endl;
			}

			/* ��ȡ�ǵ� */
			if (0 == findChessboardCorners(imageInput, board_size, image_points_buf))
			{
				cout << "can not find chessboard corners!\n"; //�Ҳ����ǵ�
				exit(1);
			}
			else
			{
				Mat view_gray;
				cvtColor(imageInput, view_gray, CV_RGB2GRAY);
				/* �����ؾ�ȷ�� */
				find4QuadCornerSubpix(view_gray, image_points_buf, Size(11, 11)); //�Դ���ȡ�Ľǵ���о�ȷ��
				image_points_seq.push_back(image_points_buf);  //���������ؽǵ�
				/* ��ͼ������ʾ�ǵ�λ�� */
				drawChessboardCorners(view_gray, board_size, image_points_buf, true); //������ͼƬ�б�ǽǵ�	
			}
		}
		
		int total = image_points_seq.size();
		cout << "total = " << total << std::endl;
		int CornerNum = board_size.width*board_size.height;  //ÿ��ͼƬ���ܵĽǵ���
		for (int ii = 0; ii<total; ii++)
		{
			if (0 == ii%CornerNum)// 24 ��ÿ��ͼƬ�Ľǵ���������ж������Ϊ����� ͼƬ�ţ����ڿ���̨�ۿ� 
			{
				int i = -1;
				i = ii / CornerNum;
				int j = i + 1;
				cout << "--> �� " << j << "ͼƬ������ --> : " << std::endl;
			}
			if (0 == ii % 3)	// ���ж���䣬��ʽ����������ڿ���̨�鿴
			{
				cout << std::endl;
			}
			else
			{
				cout.width(10);
			}

			//������еĽǵ�
			cout << " -->" << image_points_seq[ii][0].x;
			cout << " -->" << image_points_seq[ii][0].y;
		}
		cout << "�ǵ���ȡ��ɣ�\n";


		//������������궨

		cout << "��ʼ�궨������������";
		/*������ά��Ϣ*/
		Size square_size = Size(90, 90);  /* ʵ�ʲ����õ��ı궨����ÿ�����̸�Ĵ�С */
		vector<vector<Point3f>> object_points; /* ����궨���Ͻǵ����ά���� */
		/*�������*/
		Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* ������ڲ������� */
		vector<int> point_counts;  // ÿ��ͼ���нǵ������
		Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0)); /* �������5������ϵ����k1,k2,p1,p2,k3 */
		vector<Mat> tvecsMat;  /* ÿ��ͼ�����ת���� */
		vector<Mat> rvecsMat; /* ÿ��ͼ���ƽ������ */
		/* ��ʼ���궨���Ͻǵ����ά���� */
		int i, j, t;
		for (t = 0; t<image_count; t++)
		{
			vector<Point3f> tempPointSet;
			for (i = 0; i<board_size.height; i++)
			{
				for (j = 0; j<board_size.width; j++)
				{
					Point3f realPoint;
					/* ����궨�������������ϵ��z=0��ƽ���� */
					realPoint.x = i*square_size.width;
					realPoint.y = j*square_size.height;
					realPoint.z = 0;
					tempPointSet.push_back(realPoint);
				}
			}
			object_points.push_back(tempPointSet);
		}
		/* ��ʼ��ÿ��ͼ���еĽǵ��������ٶ�ÿ��ͼ���ж����Կ��������ı궨�� */
		for (i = 0; i<image_count; i++)
		{
			point_counts.push_back(board_size.width*board_size.height);
		}
		/* ��ʼ�궨 */
		calibrateCamera(object_points, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);
		//	cout << "�궨��ɣ�\n";

		//�Ա궨�����������
		cout << "��ʼ���۱궨���������������\n";
		double total_err = 0.0; /* ����ͼ���ƽ�������ܺ� */
		double err = 0.0; /* ÿ��ͼ���ƽ����� */
		vector<Point2f> image_points2; /* �������¼���õ���ͶӰ�� */
		//cout << "\tÿ��ͼ��ı궨��\n";
		//fout << "ÿ��ͼ��ı궨��\n";
		for (i = 0; i<image_count; i++)
		{
			vector<Point3f> tempPointSet = object_points[i];
			/* ͨ���õ������������������Կռ����ά���������ͶӰ���㣬�õ��µ�ͶӰ�� */
			projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points2);
			/* �����µ�ͶӰ��;ɵ�ͶӰ��֮������*/
			vector<Point2f> tempImagePoint = image_points_seq[i];
			Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
			Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
			for (int j = 0; j < tempImagePoint.size(); j++)
			{
				image_points2Mat.at<Vec2f>(0, j) = Vec2f(image_points2[j].x, image_points2[j].y);
				tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
			}
			err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
			total_err += err /= point_counts[i];
		}
		//���涨����  	
		//std::cout << "��ʼ���涨����������������" << std::endl;
		Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* ����ÿ��ͼ�����ת���� */
		fout << "����ڲ�������" << std::endl;
		fout << cameraMatrix << std::endl << std::endl;
		fout << std::endl;
		QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("�궨����������洢���ļ�caliberation_result.txt��"));
	}
	else
	{
		return;
	}
}

void StitchingVSQt::finishSetting(){

	features_type = ui->comboBox1->currentText().toStdString();
	ba_cost_func = QString(ui->comboBox2->currentText()).toStdString();
	warp_type = QString(ui->comboBox3->currentText()).toStdString();
	seam_find_type = QString(ui->comboBox4->currentText()).toStdString();
	blend_strength = ui->horizontalSlider->value();

	ui->graphicsView->setVisible(true);
//	ui->pushButton->setVisible(true);
	ui->frame->setVisible(false);
}

void StitchingVSQt::back(){
	ui->graphicsView->setVisible(true);
//	ui->pushButton->setVisible(true);
	ui->frame->setVisible(false);
}