
#include "stdafx.h"
#include "Source.h"
#include <Kinect.h>
#include <Kinect.VisualGestureBuilder.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <tchar.h>
#include <Windows.h>



using namespace std;
using namespace cv;

class Statistics
{
	float data[3];

	public: 
		Statistics()
		{
			for (int i = 0; i < 3; i++)
			{
				data[i] = 0.0;
			}
		}
	void assign (float data2[])
	{
		for (int i = 0; i < 3; i++)
		{
			data[i] = data2[i];
		}
	}

	float getMean()
	{
		float sum = 0.0;
		for (float a : data)
			sum += a;
		return sum / 3;
	}

	float getVariance()
	{
		float mean = getMean();
		float temp = 0;
		for (float a : data)
			temp += (mean - a)*(mean - a);
		return temp / 3;
	}

};


template<class Interface>
inline void SafeRelease( Interface *& pInterfaceToRelease )
{
	if( pInterfaceToRelease != NULL ){
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

int _tmain( int argc, _TCHAR* argv[] )
{
	cv::setUseOptimized( true ); 
	// Sensor
	//Statistics stat;
	IKinectSensor* pSensor;
	HRESULT hResult = S_OK;
	hResult = GetDefaultKinectSensor( &pSensor );
	if( FAILED( hResult ) ){
		std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
		return -1;
	}

	hResult = pSensor->Open();
	if( FAILED( hResult ) ){
		std::cerr << "Error : IKinectSensor::Open()" << std::endl;
		return -1;
	}

	// Source
	IColorFrameSource* pColorSource;
	hResult = pSensor->get_ColorFrameSource( &pColorSource );
	if( FAILED( hResult ) ){
		std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
		return -1;
	}

	IBodyFrameSource* pBodySource;
	hResult = pSensor->get_BodyFrameSource( &pBodySource );
	if( FAILED( hResult ) ){
		std::cerr << "Error : IKinectSensor::get_BodyFrameSource()" << std::endl;
		return -1;
	}



	// Reader
	IColorFrameReader* pColorReader;
	hResult = pColorSource->OpenReader( &pColorReader );
	if( FAILED( hResult ) ){
		std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
		return -1;
	}

	IBodyFrameReader* pBodyReader;
	hResult = pBodySource->OpenReader( &pBodyReader );
	if( FAILED( hResult ) ){
		std::cerr << "Error : IBodyFrameSource::OpenReader()" << std::endl;
		return -1;
	}



	// Description
	IFrameDescription* pDescription;
	hResult = pColorSource->get_FrameDescription( &pDescription );
	if( FAILED( hResult ) ){
		std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
		return -1;
	}

	int width = 0;
	int height = 0;
	float hand_x = 0.0, hand_y = 0.0, hand_z = 0.0;
	float el_x = 0.0, el_y = 0.0, el_z = 0.0;
	int orientation = 0;
	int gotColorFrame = 0;

	cv::Mat imgHSV;
	cv::Mat imgThresholded	;
	unsigned long avg = 0;
	int avg_size;

	vector<Point> trash_coor;
	vector<Point> hand_coor;

	Statistics stat[2];

	pDescription->get_Width( &width ); // 1920
	pDescription->get_Height( &height ); // 1080
	unsigned int bufferSize = width * height * 4 * sizeof( unsigned char );

	cv::Mat bufferMat( height, width, CV_8UC4 );
	cv::Mat bodyMat( height / 2, width / 2, CV_8UC4 );
	//cv::Mat bodyMat(height, width, CV_8UC4);

	cv::namedWindow("Color");
	cv::namedWindow("Control", CV_WINDOW_AUTOSIZE);
	cv::namedWindow( "Gesture" );
	cv::namedWindow("Last Frame");

	// Color Table
	cv::Vec3b color[BODY_COUNT];
	color[0] = cv::Vec3b( 255, 0, 0 );
	color[1] = cv::Vec3b( 0, 255, 0 );
	color[2] = cv::Vec3b( 0, 0, 255 );
	color[3] = cv::Vec3b( 255, 255, 0 );
	color[4] = cv::Vec3b( 255, 0, 255 );
	color[5] = cv::Vec3b( 0, 255, 255 );
	
	// Coordinate Mapper
	ICoordinateMapper* pCoordinateMapper;
	hResult = pSensor->get_CoordinateMapper( &pCoordinateMapper );
	if( FAILED( hResult ) ){
		std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
		return -1;
	}

	IVisualGestureBuilderFrameSource* pGestureSource[BODY_COUNT];
	IVisualGestureBuilderFrameReader* pGestureReader[BODY_COUNT];
	for( int count = 0; count < BODY_COUNT; count++ ){
		// Source
		hResult = CreateVisualGestureBuilderFrameSource( pSensor, 0, &pGestureSource[count] );
		if( FAILED( hResult ) ){
			std::cerr << "Error : CreateVisualGestureBuilderFrameSource()" << std::endl;
			return -1;
		}

		// Reader
		hResult = pGestureSource[count]->OpenReader( &pGestureReader[count] );
		if( FAILED( hResult ) ){
			std::cerr << "Error : IVisualGestureBuilderFrameSource::OpenReader()" << std::endl;
			return -1;
		}
	}

	// Create Gesture Database from File (*.gba)
	IVisualGestureBuilderDatabase* pGestureDatabase;
	hResult = CreateVisualGestureBuilderDatabaseInstanceFromFile( L"HandUp.gba"/*L"Swipe.gba"*/, &pGestureDatabase );
	if( FAILED( hResult ) ){
		std::cerr << "Error : CreateVisualGestureBuilderDatabaseInstanceFromFile()" << std::endl;
		return -1;
	}

	// Add Gesture
	UINT gestureCount = 0;
	hResult = pGestureDatabase->get_AvailableGesturesCount( &gestureCount );
	if( FAILED( hResult ) || !gestureCount ){
		std::cerr << "Error : IVisualGestureBuilderDatabase::get_AvailableGesturesCount()" << std::endl;
		return -1;
	}

	IGesture* pGesture;
	hResult = pGestureDatabase->get_AvailableGestures( gestureCount, &pGesture );
	if( SUCCEEDED( hResult ) && pGesture != nullptr ){
		
		for( int count = 0; count < BODY_COUNT; count++ ){
			hResult = pGestureSource[count]->AddGesture( pGesture );
			if( FAILED( hResult ) ){
				std::cerr << "Error : IVisualGestureBuilderFrameSource::AddGesture()" << std::endl;
				return -1;
			}

			hResult = pGestureSource[count]->SetIsEnabled( pGesture, true );
			if( FAILED( hResult ) ){
				std::cerr << "Error : IVisualGestureBuilderFrameSource::SetIsEnabled()" << std::endl;
				return -1;
			}
		}
	}
	
	while(1){
		// Frame
		IColorFrame* pColorFrame = nullptr;
		
			hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
		
		if( SUCCEEDED( hResult ) ){
			hResult = pColorFrame->CopyConvertedFrameDataToArray( bufferSize, reinterpret_cast<BYTE*>( bufferMat.data ), ColorImageFormat::ColorImageFormat_Bgra );
			if( SUCCEEDED( hResult ) ){
				cv::resize( bufferMat, bodyMat, cv::Size(), 0.5, 0.5 );
			}
		}
		SafeRelease( pColorFrame );
		
		cv::imshow("Color", bodyMat);

		cvtColor(bodyMat, imgHSV, cv::COLOR_BGR2HSV);

		inRange(imgHSV, cv::Scalar(38, 126, 42), cv::Scalar(103, 255, 255), imgThresholded); //Threshold the image tp recognize green color of the trash can.
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		

		IBodyFrame* pBodyFrame = nullptr;
		hResult = pBodyReader->AcquireLatestFrame( &pBodyFrame );
		if( SUCCEEDED( hResult ) ){
			IBody* pBody[BODY_COUNT] = { 0 };
			hResult = pBodyFrame->GetAndRefreshBodyData( BODY_COUNT, pBody );
			if( SUCCEEDED( hResult ) ){
				for( int count = 0; count < BODY_COUNT; count++ ){
					BOOLEAN bTracked = false;
					hResult = pBody[count]->get_IsTracked( &bTracked );
					if( SUCCEEDED( hResult ) && bTracked ){
						// Joint
						Joint joint[JointType::JointType_Count];
						hResult = pBody[count]->GetJoints( JointType::JointType_Count, joint );
						if( SUCCEEDED( hResult ) ){
							ColorSpacePoint colorSpacePoint_hand = { 0 };
							ColorSpacePoint colorSpacePoint_shoulder = { 0 };
							ColorSpacePoint colorSpacePoint_elbow = { 0 };
							pCoordinateMapper->MapCameraPointToColorSpace(joint[11].Position, &colorSpacePoint_hand);
							pCoordinateMapper->MapCameraPointToColorSpace(joint[8].Position, &colorSpacePoint_shoulder);
							pCoordinateMapper->MapCameraPointToColorSpace(joint[9].Position, &colorSpacePoint_elbow);
							int x_hand = static_cast<int>(colorSpacePoint_hand.X);
							int y_hand = static_cast<int>(colorSpacePoint_hand.Y);
							if ((x_hand >= 0) && (x_hand < width) && (y_hand >= 0) && (y_hand < height)){
								cv::circle(bufferMat, cv::Point(x_hand, y_hand), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA);
							}
							int x_shoulder = static_cast<int>(colorSpacePoint_shoulder.X);
							int y_shoulder = static_cast<int>(colorSpacePoint_shoulder.Y);
							if ((x_shoulder >= 0) && (x_shoulder < width) && (y_shoulder >= 0) && (y_shoulder < height)){
								cv::circle(bufferMat, cv::Point(x_shoulder, y_shoulder), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA);
							}
							int x_elbow = static_cast<int>(colorSpacePoint_elbow.X);
							int y_elbow = static_cast<int>(colorSpacePoint_elbow.Y);
							if ((x_elbow >= 0) && (x_elbow < width) && (y_elbow >= 0) && (y_elbow < height)){
								cv::circle(bufferMat, cv::Point(x_elbow, y_elbow), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA);
							}
							cout << joint[11].Position.X << " " << joint[11].Position.Y << " " << joint[11].Position.Z << endl;
							cout << joint[9].Position.X << " " << joint[9].Position.Y << " " << joint[9].Position.Z << endl;
							cout << joint[8].Position.X << " " << joint[8].Position.Y << " " << joint[8].Position.Z << endl;
							cout << joint[11].Position.Y - joint[9].Position.Y - joint[8].Position.Y << endl;
							cout << joint[11].Position.Z - joint[9].Position.Z - joint[8].Position.Z << endl;
							
							float a[3] = { joint[11].Position.Y, joint[9].Position.Y, joint[8].Position.Y };
							float b[3] = { joint[11].Position.Z, joint[9].Position.Z, joint[8].Position.Z };
							
							stat[0].assign(a);
							stat[1].assign(b);

							cout << stat[0].getVariance() << " " << stat[1].getVariance() << endl;
							if (((stat[0].getVariance()) < 0.007) && ((stat[1].getVariance()) < 0.007) && orientation == 0){
								cout << "Success" << endl;


								orientation = 1;
								hand_x = joint[11].Position.X;
								hand_y = joint[11].Position.Y;
								hand_z = joint[11].Position.Z;
								hand_coor.push_back(Point(x_hand, x_elbow));
							}

							
							cout << "Orientation:" << orientation << endl;

							for( int type = 0; type < JointType::JointType_Count; type++ ){
								ColorSpacePoint colorSpacePoint = { 0 };
								pCoordinateMapper->MapCameraPointToColorSpace( joint[type].Position, &colorSpacePoint );
								int x = static_cast<int>( colorSpacePoint.X );
								int y = static_cast<int>( colorSpacePoint.Y );
								if( ( x >= 0 ) && ( x < width ) && ( y >= 0 ) && ( y < height ) ){
									cv::circle( bufferMat, cv::Point( x, y ), 5, static_cast< cv::Scalar >( color[count] ), -1, CV_AA );
								}
							}
						}

						// Set TrackingID to Detect Gesture
						UINT64 trackingId = _UI64_MAX;
						hResult = pBody[count]->get_TrackingId( &trackingId );
						if( SUCCEEDED( hResult ) ){
							pGestureSource[count]->put_TrackingId( trackingId );
						}
					}
				}
				cv::resize( bufferMat, bodyMat, cv::Size(), 0.5, 0.5 );
			}
			for( int count = 0; count < BODY_COUNT; count++ ){
				SafeRelease( pBody[count] );
			}
		}
		SafeRelease( pBodyFrame );

		// Detect Gesture
		std::system( "cls" );
		for( int count = 0; count < BODY_COUNT; count++ ){
			IVisualGestureBuilderFrame* pGestureFrame = nullptr;
			hResult = pGestureReader[count]->CalculateAndAcquireLatestFrame( &pGestureFrame );
			if( SUCCEEDED( hResult ) && pGestureFrame != nullptr ){
				BOOLEAN bGestureTracked = false;
				hResult = pGestureFrame->get_IsTrackingIdValid( &bGestureTracked );
				if( SUCCEEDED( hResult ) && bGestureTracked ){
					// Discrete Gesture (Sample HandUp.gba is Action to Hand Up above the head.)
					IDiscreteGestureResult* pGestureResult = nullptr;
					hResult = pGestureFrame->get_DiscreteGestureResult( pGesture, &pGestureResult );
					if( SUCCEEDED( hResult ) && pGestureResult != nullptr ){
						BOOLEAN bDetected = false;
						hResult = pGestureResult->get_Detected( &bDetected );
						if( SUCCEEDED( hResult ) && bDetected ){
							std::cout << "Detected Gesture" << std::endl;
						}
					}

		
					SafeRelease( pGestureResult );
				}
			}
			SafeRelease( pGestureFrame );
		}
		
		if (cv::waitKey(10) == VK_ESCAPE){
			break;
		}

		if (orientation)
		{
			break;
		}

			
		
	}

	cout << "X:" << hand_x << "\tY:" << hand_y << "\tZ:" << hand_z << endl; 
	cout << "Variance_Z" << stat[1].getVariance() << "\tVariance_Y" << stat[0].getVariance() << endl;
	cout << "Got the frames" << gotColorFrame << endl;
	cv::imshow("Last Frame", bodyMat);
	cv::imshow("Thresholded Image", imgThresholded);
	avg_size = 0;
	for (int i = 0; i < imgThresholded.rows; i++)
	{
		for (int j = 0; j < imgThresholded.cols; j++)
		{
			if (imgThresholded.at<uchar>(i,j) == 255)
			{
				avg = avg + j;
				avg_size++;
			}
		}
	}

	avg = avg / avg_size;
	cout << avg << endl;
	float bot_x, distance,time, focal_length;
	focal_length = abs((hand_coor[0].x * hand_z) / hand_x);
	cout << "f:" << focal_length << endl;
	
	if (avg <= 490 )
	{
		bot_x = (avg*hand_z) / 2182;
		distance = sqrt(pow((bot_x + hand_x), 2));
		distance = distance;
	} 
	else
	{
		bot_x = (avg*hand_z) / 2182;
		distance = sqrt(pow((-bot_x + hand_x), 2));
		distance = distance + 0.20;
	}
	
	cout << bot_x << endl;
	time = (int)((distance / 0.67)*1000);
	char t[50];
	_itoa_s(time, t, 10);
	cout << "Distance" << distance << "\tTime" << time << endl;
    bluetooth(t);

	while (1){
		if (cv::waitKey(10) == VK_ESCAPE){
			break;
		}
	}
	
	SafeRelease( pColorSource );
	SafeRelease( pBodySource );
	SafeRelease( pColorReader );
	SafeRelease( pBodyReader );
	SafeRelease( pDescription );
	SafeRelease( pCoordinateMapper );
	for( int count = 0; count < BODY_COUNT; count++ ){
		SafeRelease( pGestureSource[count] );
		SafeRelease( pGestureReader[count] );
	}
	SafeRelease( pGestureDatabase );
	SafeRelease( pGesture );
	if( pSensor ){
		pSensor->Close();
	}
	SafeRelease( pSensor );
	//cv::destroyAllWindows();

	return 0;
}

