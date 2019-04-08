/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#include<stdlib.h>
#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>

#include<opencv2/core/core.hpp>

#include<System.h>

using namespace std;
// using namespace cv;
/*
void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps);
*/
int main(int argc, char **argv)
{
    if(argc != 5)
    {
        cerr << endl << "Usage: ./mono_tum path_to_vocabulary path_to_settings video_file start_timestamp" << endl;
        return 1;
    }

    cv::VideoCapture cap(argv[3]);
    if (!cap.isOpened()){
        cerr << "ERROR: Failed to load video" << endl;
        return 1;
    }
    // 帧率
    double rate = cap.get(CV_CAP_PROP_FPS);
    int nImages = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
    int width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);

/*
    // Retrieve paths to images
    vector<string> vstrImageFilenames;
    vector<double> vTimestamps;
    LoadImages(string(argv[3]), string(argv[4]), vstrImageFilenames, vTimestamps);

    int nImages = vstrImageFilenames.size();
*/
    if(nImages<=0)
    {
        cerr << "ERROR: Failed to load images" << endl;
        return 1;
    }

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::MONOCULAR,true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    cout << "Images in the sequence: " << nImages << endl << endl;

    cv::Mat frame;
    double tframe = atof(argv[4])/1000.0;
    int ni = 0;
    while(cap.read(frame)) {
        cv::Mat im;
        cv::resize(frame, im, cv::Size(960,640));

    // }
    // // Main loop
    // cv::Mat im;
    // for(int ni=0; ni<nImages; ni++)
    // {
    //     // Read image from file
    //     im = cv::imread(vstrImageFilenames[ni],CV_LOAD_IMAGE_UNCHANGED);
    //     double tframe = vTimestamps[ni];

    //     if(im.empty())
    //     {
    //         cerr << endl << "Failed to load image at: "
    //              <<  vstrImageFilenames[ni] << endl;
    //         return 1;
    //     }

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the image to the SLAM system
        SLAM.TrackMonocular(im,tframe);

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

        vTimesTrack[ni]=ttrack;

        // Wait to load the next frame
        double T=0;
        // if(ni<nImages-1)
        //     T = vTimestamps[ni+1]-tframe;
        // else if(ni>0)
        //     T = tframe-vTimestamps[ni-1];
        T = 1.0/CV_CAP_PROP_FPS;

        if(ttrack<T)
            usleep((T-ttrack)*1e6);
        
        tframe += 1.0/CV_CAP_PROP_FPS;
        ni++;
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        totaltime+=vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
    cout << "mean tracking time: " << totaltime/nImages << endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

    return 0;
}
/*
void LoadImages(const string &strImagePath, const string &strPathTimes,
                vector<string> &vstrImages, vector<double> &vTimeStamps)
{
    ifstream fTimes;
    fTimes.open(strPathTimes.c_str());
    vTimeStamps.reserve(5000);
    vstrImages.reserve(5000);
    while(!fTimes.eof())
    {
        string s;
        getline(fTimes,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            vstrImages.push_back(strImagePath + "/" + ss.str() + ".png");
            double t;
            ss >> t;
            vTimeStamps.push_back(t/1e9);

        }
    }
}
*/