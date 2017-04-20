#include "stdafx.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

//------------ ���N�ⲿ���� ----------//
double first[12][2] =
{
	{ 488.362, 169.911 },
	{ 449.488, 174.44 },
	{ 408.565, 179.669 },
	{ 364.512, 184.56 },
	{ 491.483, 122.366 },
	{ 451.512, 126.56 },
	{ 409.502, 130.342 },
	{ 365.5, 134 },
	{ 494.335, 74.544 },
	{ 453.5, 76.5 },
	{ 411.646, 79.5901 },
	{ 366.498, 81.6577 }
};

double second[12][2] =
{
	{ 526.605, 213.332 },
	{ 470.485, 207.632 },
	{ 417.5, 201 },
	{ 367.485, 195.632 },
	{ 530.673, 156.417 },
	{ 473.749, 151.39 },
	{ 419.503, 146.656 },
	{ 368.669, 142.565 },
	{ 534.632, 97.5152 },
	{ 475.84, 94.6777 },
	{ 421.16, 90.3223 },
	{ 368.5, 87.5 }
};



void main()
{
	//����궨��������
	double K[3][3] = {10494.14648730224, 0, 2276.545279477376, 0, 10588.78849345028, 1543.138721764276, 0, 0, 1 };    //������ڲ�������K
	cv::Mat cameraMatrix = cv::Mat(3, 3, CV_64FC1, K);  //����궨����
	CvMat *transCamera = cvCreateMat(3, 3, CV_64FC1);
	CvMat temp = cameraMatrix;
	cvCopy(&temp, transCamera);  //matתCvMat

	cout << "����������������" << endl;
	for (int y = 0; y < 3; ++y){
		for (int x = 0; x < 3; ++x)
		{
			std::cout << CV_MAT_ELEM(*transCamera, double, y, x) << " ";
		}
		std::cout << endl;
	}

	CvMat *firstM = cvCreateMat(12, 2, CV_64FC1);
	cvSetData(firstM, first, firstM->step);

	CvMat *secondM = cvCreateMat(12, 2, CV_64FC1);
	cvSetData(secondM, second, secondM->step);

	/*Mat copyFisrtM = Mat(firstM, true);*/
	Mat copyFirstM(firstM->rows, firstM->cols, CV_64FC1, firstM->data.fl);
	Mat copySecondM(secondM->rows, secondM->cols, CV_64FC1, secondM->data.fl);
	//�õ����ʾ���
	Mat essentialMat = findEssentialMat(copyFirstM, copySecondM,cameraMatrix, CV_FM_RANSAC, 0.999, 1.0);
	//�ֽ�
	Mat Rotation;
	Mat Translation;

	if (recoverPose(essentialMat, copyFirstM, copySecondM, cameraMatrix, Rotation, Translation)){
		cout << "E:" << essentialMat << endl;
		cout << "R:" << Rotation << endl;
		cout << "T:" << Translation << endl;
	}
	else{
		cout << "�ֽ�ʧ��" << endl;
	}
	//CvMat *FMat = cvCreateMat(3, 3, CV_64FC1);//��������

	//if (cvFindFundamentalMat(firstM, secondM, FMat, CV_FM_RANSAC, 1.00, 0.99) == 0){
	//	std::cerr << "Can't Get F Mat/n";
	//	return ;
	//}

	//cout << "�������������" << endl;
	//for (int y = 0; y < 3; ++y){		
	//	for (int x = 0; x < 3; ++x)
	//	{
	//		std::cout << CV_MAT_ELEM(*FMat, double, y, x) << " ";
	//	}
	//	std::cout << endl;
	//}

	//CvMat *EMat = cvCreateMat(3, 3, CV_64FC1);

	//CvMat *Transmat = cvCreateMat(3, 3, CV_64FC1);
	//cvTranspose(transCamera, Transmat);
	////���ʾ���=�������������*��������*�����������

	//CvMat *ResultMatrix1 = cvCreateMat(3, 3, CV_64FC1);
	//CvMat *ResultMatrix2 = cvCreateMat(3, 3, CV_64FC1);//���ʾ���

	//cvMul(Transmat, FMat, ResultMatrix1);
	//cvMul(ResultMatrix1,transCamera, ResultMatrix2);

	//cout << "���ʾ������:" << endl;
	//for (int y = 0; y < 3; ++y){
	//	
	//	for (int x = 0; x < 3; ++x)
	//	{
	//		std::cout << CV_MAT_ELEM(*ResultMatrix2, double, y, x) << " ";
	//	}
	//	std::cout << endl;
	//}
	////*EMat = (*FMat)*(*Transmat);
	///*Mat result2Copy = Mat(ResultMatrix2);*/
	//cv::Mat result2Copy(ResultMatrix2->rows, ResultMatrix2->cols, CV_64FC1, ResultMatrix2->data.fl);
	//SVD thissvd(result2Copy, SVD::FULL_UV);

	////[U,S,V]=svd(A)
	//Mat u = Mat(3, 3, CV_64FC1);
	//Mat s = Mat(3, 3, CV_64FC1);
	//Mat vt = Mat(3, 3, CV_64FC1);//v��ת��

	//u = thissvd.u;
	//s = thissvd.w;
	//vt = thissvd.vt;

	////U��V��������໥��������,��S����һ�ԽǾ���

	//cout << "u:" << u << endl;
	//cout << "s:" << s << endl;
	//cout << "vt:" << vt << endl;

	cvReleaseMat(&firstM);
	cvReleaseMat(&secondM);
	/*cvReleaseMat(&FMat);
	cvReleaseMat(&Transmat);
	cvReleaseMat(&EMat);
	cvReleaseMat(&ResultMatrix1);
	cvReleaseMat(&ResultMatrix2);*/

	ifstream fin("calibdata.txt"); /* �궨����ͼ���ļ���·�� */
	ofstream fout("caliberation_result.txt");  /* ����궨������ļ� */
	//��ȡÿһ��ͼ�񣬴�����ȡ���ǵ㣬Ȼ��Խǵ���������ؾ�ȷ��	
	cout << "��ʼ��ȡ�ǵ㡭����������";
	int image_count = 0;  /* ͼ������ */
	Size image_size;  /* ͼ��ĳߴ� */
	Size board_size = Size(6, 9);    /* �궨����ÿ�С��еĽǵ��� */
	vector<Point2f> image_points_buf;  /* ����ÿ��ͼ���ϼ�⵽�Ľǵ� */
	vector<vector<Point2f>> image_points_seq; /* �����⵽�����нǵ� */
	string filename;
	int count = -1;//���ڴ洢�ǵ������
	while (getline(fin, filename))
	{
		image_count++;
		// ���ڹ۲�������
		cout << "image_count = " << image_count << endl;
		/* �������*/
		cout << "-->count = " << count;
		Mat imageInput = imread(filename);
		if (image_count == 1)  //�����һ��ͼƬʱ��ȡͼ������Ϣ
		{
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
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
			imshow("Camera Calibration", view_gray);//��ʾͼƬ
			waitKey(500);//��ͣ0.5S		
		}
	}
	int total = image_points_seq.size();
	cout << "total = " << total << endl;
	int CornerNum = board_size.width*board_size.height;  //ÿ��ͼƬ���ܵĽǵ���
	for (int ii = 0; ii<total; ii++)
	{
		if (0 == ii%CornerNum)// 24 ��ÿ��ͼƬ�Ľǵ���������ж������Ϊ����� ͼƬ�ţ����ڿ���̨�ۿ� 
		{
			int i = -1;
			i = ii / CornerNum;
			int j = i + 1;
			cout << "--> �� " << j << "ͼƬ������ --> : " << endl;
		}
		if (0 == ii % 3)	// ���ж���䣬��ʽ����������ڿ���̨�鿴
		{
			cout << endl;
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
	cout << "�궨��ɣ�\n";

	//�Ա궨�����������
	cout << "��ʼ���۱궨���������������\n";
	double total_err = 0.0; /* ����ͼ���ƽ�������ܺ� */
	double err = 0.0; /* ÿ��ͼ���ƽ����� */
	vector<Point2f> image_points2; /* �������¼���õ���ͶӰ�� */
	cout << "\tÿ��ͼ��ı궨��\n";
	fout << "ÿ��ͼ��ı궨��\n";
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
		std::cout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ����" << err << "����" << endl;
	}
	std::cout << "����ƽ����" << total_err / image_count << "����" << endl;
	fout << "����ƽ����" << total_err / image_count << "����" << endl << endl;
	std::cout << "������ɣ�" << endl;


	//���涨����  	
	std::cout << "��ʼ���涨����������������" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* ����ÿ��ͼ�����ת���� */
	fout << "����ڲ�������" << endl;
	fout << cameraMatrix << endl << endl;
	fout << "����ϵ����\n";
	fout << distCoeffs << endl << endl << endl;
	for (int i = 0; i<image_count; i++)
	{
		fout << "��" << i + 1 << "��ͼ�����ת������" << endl;
		fout << tvecsMat[i] << endl;
		/* ����ת����ת��Ϊ���Ӧ����ת���� */
		Rodrigues(tvecsMat[i], rotation_matrix);
		fout << "��" << i + 1 << "��ͼ�����ת����" << endl;
		fout << rotation_matrix << endl;
		fout << "��" << i + 1 << "��ͼ���ƽ��������" << endl;
		fout << rvecsMat[i] << endl << endl;
	}
	std::cout << "��ɱ���" << endl;
	fout << endl;
	system("pause");



	//��ʾ������  
	 	Mat mapx = Mat(image_size,CV_32FC1);
	 	Mat mapy = Mat(image_size,CV_32FC1);
	 	Mat R = Mat::eye(3,3,CV_32F);
	 	std::cout<<"�������ͼ��"<<endl;
	 	string imageFileName;
	 	std::stringstream StrStm;
	 	for (int i = 0 ; i != image_count ; i++)
	 	{
	 		std::cout<<"Frame #"<<i+1<<"..."<<endl;
	 		Mat newCameraMatrix = Mat(3,3,CV_32FC1,Scalar::all(0));
	 		initUndistortRectifyMap(cameraMatrix,distCoeffs,R,cameraMatrix,image_size,CV_32FC1,mapx,mapy);
	 		StrStm.clear();
	 		imageFileName.clear();
	 		StrStm<<i+1;
	 		StrStm>>imageFileName;
	 		imageFileName += ".jpg";
	 		Mat t = imread(imageFileName);
	 		Mat newimage = t.clone();
	 		cv::remap(t,newimage,mapx, mapy, INTER_LINEAR);
	 		StrStm.clear();
	 		imageFileName.clear();
	 		StrStm<<i+1;
	 		StrStm>>imageFileName;
	 		imageFileName += "_d.jpg";
	 		imwrite(imageFileName,newimage);
	 	}
	 	std::cout<<"�������"<<endl;
	return;
}