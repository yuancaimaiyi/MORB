/*
 #############################################################################
 #        Author: Alessio Xompero
 #         Email: a.xompero@qmul.ac.uk
 #
 #
 #  Created Date: 2017/11/14
 # Modified Date: 2021/04/10
 #
 # Centre for Intelligent Sensing, Queen Mary University of London, UK
 #
 ###############################################################################
 #
 # The MIT License (MIT)
 #
 # Copyright © 2021 Alessio Xompero
 #
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the “Software”), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 # copies of the Software, and to permit persons to whom the Software is
 # furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included in
 # all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 # SOFTWARE.
 #
 ###############################################################################
 */

#include <bitset>

// My libraries
#include "utilities.h"

void PrintHeading1(const std::string& heading) {
  std::cout << std::endl << std::string(78, '=') << std::endl;
  std::cout << heading << std::endl;
  std::cout << std::string(78, '=') << std::endl << std::endl;
}

void PrintHeading2(const std::string& heading) {
  int num_symb = (78 - heading.length()) / 2;
  std::cout << std::endl << std::string(78, '#') << std::endl;
  std::cout << std::string(num_symb, '#') << heading
            << std::string(num_symb, '#') << std::endl;
}

void PrintClosing2(const std::string& heading) {
  int num_symb = (78 - heading.length()) / 2;
  std::cout << std::string(num_symb, '#') << heading
            << std::string(num_symb, '#') << std::endl;
  std::cout << std::string(78, '#') << std::endl << std::endl;
}

// Read the key-points provided by Heinly for the feature evaluation
void readFeatures(std::string filename, std::vector<cv::Point2f> &feats) {
  std::cout << "Reading key-points" << std::endl;
  feats.clear();

  // Open the key-point file provided by Heinly.
  std::ifstream fMapPoints(filename.c_str());

  if (!fMapPoints.is_open()) {
    std::string msg = "Error opening file" + filename;
    perror(msg.c_str());
    return;
  }

  std::string line;  //Container for the line to read

  cv::Point2f kpt;

  while (getline(fMapPoints, line)) {
    std::istringstream in(line);
    in >> kpt.x;
    in >> kpt.y;
    feats.push_back(kpt);
  }

  fMapPoints.close();
}

//
void readFeatures(std::string filename, std::vector<cv::Point2f> &KeyPointsLoc,
                  cv::OutputArray KeyPointsDesc) {
  KeyPointsLoc.clear();

  // Open the MapPoint file generated by ORB-SLAM.
  std::ifstream fMapPoints(filename.c_str());

  if (!fMapPoints.is_open()) {
    std::string msg = "Error opening file" + filename;
    perror(msg.c_str());
    return;
  }

  std::string line;  //Container for the line to read

  getline(fMapPoints, line);
  std::istringstream in(line);
  int descSize;
  in >> descSize;
  getline(fMapPoints, line);

  cv::Point2f kpt;
  cv::Mat D = KeyPointsDesc.getMat_();

  while (getline(fMapPoints, line)) {
    std::istringstream in(line);
    in >> kpt.x;
    in >> kpt.y;
    KeyPointsLoc.push_back(kpt);

    cv::Mat tmp(1, descSize, CV_32SC1);
    for (int s = 0; s < descSize; s += 4) {
      in >> tmp.at<int>(0, s);
      in >> tmp.at<int>(0, s + 1);
      in >> tmp.at<int>(0, s + 2);
      in >> tmp.at<int>(0, s + 3);
    }

    D.push_back(tmp);
  }

  fMapPoints.close();

  // Check that the number of descriptors corresponds to the number of
  // points declared at the beginning of the file
  assert(KeyPointsLoc.size() == D.rows);

  D.convertTo(KeyPointsDesc, CV_8UC1);
}

//
void saveFeatures(std::vector<cv::Point2f> KeyPointsLoc,
                  cv::InputArray KeyPointsDesc, std::string output) {
  std::cout << "Saving features.." << std::endl;

  cv::Mat D = KeyPointsDesc.getMat_();

  CV_Assert(D.rows == (int ) KeyPointsLoc.size());

  int M = D.rows;
  int S = D.cols;

  std::ofstream fKeyPoints(output.c_str());
  if (!fKeyPoints.is_open()) {
    std::string msg = "Error opening file" + output;
    perror(msg.c_str());
    return;
  }

  fKeyPoints << D.cols << "\n";
  fKeyPoints << M << "\n";
  D.convertTo(D, CV_32SC1);

  for (int j = 0; j < M; ++j) {
    fKeyPoints << KeyPointsLoc[j].x << " " << KeyPointsLoc[j].y << " ";

    for (int s = 0; s < S; s += 4) {
      fKeyPoints << D.at<int>(j, s) << " ";
      fKeyPoints << D.at<int>(j, s + 1) << " ";
      fKeyPoints << D.at<int>(j, s + 2) << " ";
      fKeyPoints << D.at<int>(j, s + 3) << " ";
    }
    fKeyPoints << "\n";
  }

  fKeyPoints.close();
}

//
bool writeMatches(cv::InputArray _matches, std::string matchesFileName) {
  cv::Mat matches = _matches.getMat();

  FILE *fout;
  if ((fout = fopen(matchesFileName.c_str(), "w")) == NULL) {
    std::cout << "Could not open" + matchesFileName + " for writing."
              << std::endl;
    return false;
  }

  int N = matches.rows;  // Number of matches

  for (int n = 0; n < N; ++n) {
    if (matches.type() == 4) {
      fprintf(fout, "%d %d %d\n", matches.at<int>(n, 0), matches.at<int>(n, 1),
              matches.at<int>(n, 2));
    } else {
      fprintf(fout, "%.3f %.3f %.3f\n", matches.at<float>(n, 0),
              matches.at<float>(n, 1), matches.at<float>(n, 2));
    }
  }

  fclose(fout);

  return true;
}

bool checkDuplicates(cv::InputArray _V) {
  cv::Mat C1 = _V.getMat();

  assert(C1.rows == 1 || C1.cols == 1);

  if (C1.rows == 1) {
    C1 = C1.t();
  }

  std::vector<float> dupl;
  for (size_t n = 0; n < C1.rows; ++n) {
    dupl.insert(dupl.end(), C1.ptr<float>(n), C1.ptr<float>(n) + C1.cols);
  }

  // Sort the vector if not sorted
  if (!std::is_sorted(dupl.begin(), dupl.end())) {
    std::sort(dupl.begin(), dupl.end());
  }

  int nDupl = 0;
  float vD = -1;
  for (int j = 0; j < dupl.size(); ++j) {
    if (dupl[j] == vD) {
      ++nDupl;
    }

    vD = dupl[j];
  }

  return nDupl > 0;
}

void saveKeyPoints(
    std::string output,
    const std::vector<std::vector<cv::KeyPoint> >& allKeypoints) {
  std::ofstream fKeyPoints(output.c_str());
  if (!fKeyPoints.is_open()) {
    std::string msg = "Error opening file" + output;
    perror(msg.c_str());
    return;
  }

  size_t L = allKeypoints.size();
  for (size_t l = 0; l < L; ++l) {
    std::vector<cv::KeyPoint> kpsLevel = allKeypoints[l];

    std::vector<cv::KeyPoint>::iterator iti, ite;
    iti = kpsLevel.begin();
    ite = kpsLevel.end();
    for (; iti != ite; ++iti) {
      fKeyPoints << iti->pt.x << " ";
      fKeyPoints << iti->pt.y << " ";
      fKeyPoints << iti->octave << " ";
      fKeyPoints << iti->angle << " ";
      fKeyPoints << iti->size << " ";
      fKeyPoints << iti->response << "\n";
    }
  }

  fKeyPoints.close();
}

void saveKeyPoints(std::string output,
                   const std::vector<std::vector<cv::KeyPoint> >& allKeypoints,
                   std::vector<int> dtScales) {
  std::ofstream fKeyPoints(output.c_str());
  if (!fKeyPoints.is_open()) {
    std::string msg = "Error opening file" + output;
    perror(msg.c_str());
    return;
  }

  size_t N = allKeypoints[0].size();

  int l = 0;
  std::vector<cv::KeyPoint> kpsLevel = allKeypoints[0];
  for (size_t n = 0; n < N; ++n) {
    if (dtScales[n] != l) {
      l = dtScales[n];
      kpsLevel = allKeypoints[l];
    }

    fKeyPoints << kpsLevel[n].pt.x << " ";
    fKeyPoints << kpsLevel[n].pt.y << " ";
    fKeyPoints << kpsLevel[n].octave << " ";
    fKeyPoints << kpsLevel[n].angle << " ";
    fKeyPoints << kpsLevel[n].size << " ";
    fKeyPoints << kpsLevel[n].response << "\n";
  }

  fKeyPoints.close();
}

void readKeyPoints(std::string filename,
                   std::vector<cv::KeyPoint>& allKeypoints) {
  allKeypoints.clear();

  std::ifstream fKeyPoints(filename.c_str());

  if (!fKeyPoints.is_open()) {
    std::string msg = "Error opening file" + filename;
    perror(msg.c_str());
    return;
  }

  std::string line;  //Container for the line to read

  while (getline(fKeyPoints, line)) {
    std::istringstream in(line);

    cv::KeyPoint kpt;
    in >> kpt.pt.x;
    in >> kpt.pt.y;
    in >> kpt.octave;
    in >> kpt.size;
    in >> kpt.response;
    in >> kpt.angle;

    allKeypoints.push_back(kpt);
  }

  fKeyPoints.close();
}

void readHomography(std::string filename, cv::OutputArray _H) {
  std::cout << "Reading homography.." << std::endl;
  std::cout << filename << std::endl;

  std::ifstream fMapPoints(filename.c_str());

  if (!fMapPoints.is_open()) {
    std::string msg = "Error opening file" + filename;
    perror(msg.c_str());
    return;
  }

  std::string line;  //Container for the line to read

  cv::Mat H(3, 3, CV_64FC1);

  getline(fMapPoints, line);
  std::istringstream in1(line);
  in1 >> H.at<double>(0, 0);
  in1 >> H.at<double>(0, 1);
  in1 >> H.at<double>(0, 2);

  getline(fMapPoints, line);
  std::istringstream in2(line);
  in2 >> H.at<double>(1, 0);
  in2 >> H.at<double>(1, 1);
  in2 >> H.at<double>(1, 2);

  getline(fMapPoints, line);
  std::istringstream in3(line);
  in3 >> H.at<double>(2, 0);
  in3 >> H.at<double>(2, 1);
  in3 >> H.at<double>(2, 2);

  fMapPoints.close();

  H.convertTo(H, CV_32FC1);
  //std::cout << H << std::endl;

  H.copyTo(_H);
}

////////////////////////////////////////////////////////////////////////////////
// Get the distance and offset of the two points
/**
 * @param s1  Detection scale of the feature in the first image.
 * @param s2  Detection scale of the feature in the second image.
 * @param WMA Boolean variable to determine if also computing weights.
 * @param SEC Boolean variable to determine if computing only the single scale
 *            descriptor distances at resolution layers higher than the detection
 *            scale (i.e. coarser levels).
 * @param W   Half-length of the diagonal for the cross-correlation based distance.
 * @param R   Variable that denotes if to compute only the value at the detection
 *            scale (R=-1), if only at lower scales than the detection scale of
 *            either one of the two features (R=0), or to find the minimum in
 *            all possible cross scales between features. When R
 */
void getDistanceAndScaleRatio(cv::InputArray _block, std::vector<int>& scores,
                              int s1, int s2, const bool& WMA, const bool& SEC,
                              const int& W, const int& R) {
  int v = 1;

  scores.clear();
  scores.resize(3);

  cv::Mat block = _block.getMat();

  cv::Mat K = cv::Mat::eye(W * 2 + 1, W * 2 + 1, block.type());  // Diagonal kernel
  K.convertTo(K, CV_32FC1);

  if (SEC && WMA) {
    cv::Mat tmp = K.rowRange(0, W).colRange(0, W);
    tmp.setTo(0);

    double a, tot = 0;
    ;
    std::vector<float> q;
    for (int j = 0; j < W + 1; ++j) {
      //std::cout << j/ (float)(L + 1) << std::endl;
      //std::cout << pow(j/ (float) (L + 1), 2) << std::endl;
      //std::cout << 1 - pow(j/ (float)(L + 1), 2) << std::endl;
      a = pow(1 - pow(j / (float) (W + 1), 2), 2);
      //std::cout << a << std::endl;
      q.push_back(a);
      tot += a;
    }

    for (int j = W; j < 2 * W + 1; ++j) {
      //std::cout << q[j-L] << " / " << tot << std::endl;
      K.at<float>(j, j) = q[j - W] / tot;
    }
  } else if (SEC) {
    cv::Mat tmp = K.rowRange(0, W).colRange(0, W);
    tmp.setTo(0);
  } else if (WMA) {
    double a, tot = 0;
    ;
    std::vector<float> q;
    for (int j = -W; j < W + 1; ++j) {
      a = pow(1 - pow(j / (float) (W + 1), 2), 2);
      q.push_back(a);
      tot += a;
    }

    for (int j = 0; j < 2 * W + 1; ++j) {
      K.at<float>(j, j) = q[j] / tot;
    }
  }

  cv::Mat dummyOnes = cv::Mat::ones(block.rows, block.cols, block.type());

  cv::Mat fBlock, normFact;

  block.convertTo(block, CV_32FC1);
  dummyOnes.convertTo(dummyOnes, CV_32FC1);

  cv::filter2D(block, fBlock, -1, K, cv::Point(-1, -1), 0, cv::BORDER_ISOLATED);  // Convolution of the Hamming distance matrix with the diagonal kernel
  cv::filter2D(dummyOnes, normFact, -1, K, cv::Point(-1, -1), 0,
               cv::BORDER_ISOLATED);  // Convolution of the Hamming distance matrix with the diagonal kernel

  if (v == 1) {
    std::cout << block << std::endl;
    std::cout << "Kernel:" << std::endl;
    std::cout << K << std::endl;

    std::cout << "Block after conv:" << std::endl;
    std::cout << fBlock << std::endl;

    std::cout << "Normalisation factor:" << std::endl;
    std::cout << normFact << std::endl;
  }

  cv::divide(fBlock, normFact, fBlock);

  dummyOnes *= 256;
  if (R == -1) {
    dummyOnes.at<float>(s1, s2) = 0;
  } else if (R == 0) {
    cv::Mat row0 = dummyOnes.row(s1).colRange(s2, dummyOnes.cols);
    row0.setTo(0);

    cv::Mat col0 = dummyOnes.col(s2).rowRange(s1, dummyOnes.rows);
    col0.setTo(0);
  } else {
    cv::Mat area = dummyOnes.rowRange(s1, dummyOnes.rows).colRange(
        s2, dummyOnes.cols);
    area.setTo(0);
  }

  cv::Mat roi = fBlock + dummyOnes;

  double minHblock;
  cv::Point idx;
  cv::minMaxLoc(roi, &minHblock, 0, &idx);

  scores[0] = (int) round(minHblock);  // It would be better ceil as done for MORB-D
  scores[1] = (int) idx.y;
  scores[2] = (int) idx.x;

  if (v == 1) {
    std::cout << "Block after conv and normalisation:" << std::endl;
    std::cout << fBlock << std::endl;
    std::cout << "Mask:" << std::endl;
    std::cout << dummyOnes << std::endl;
    std::cout << "Masked block:" << std::endl;
    std::cout << roi << std::endl;

    std::cout << scores[0] << ", " << scores[1] << ", " << scores[2]
              << std::endl;

    std::cout << "Press button to continue..." << std::endl;
    std::cin.get();
  }
}

void saveFeatures(std::vector<cv::KeyPoint> KeyPointsLoc,
                  cv::InputArray KeyPointsDesc, std::string output,
                  distance_type dt, int nOctaves) {
  std::cout << "Saving features.." << std::endl;

  cv::Mat D = KeyPointsDesc.getMat_();

  CV_Assert(D.rows == (int ) KeyPointsLoc.size());

  int M = D.rows;
  int S = D.cols;

  std::ofstream fKeyPoints(output.c_str());
  if (!fKeyPoints.is_open()) {
    std::string msg = "Error opening file" + output;
    perror(msg.c_str());
    return;
  }

  // Descriptor length (bytes for floating point, bits for binary)
  switch (dt) {
    case L1_NORM:
      fKeyPoints << S << "\n";
      break;
    case L2_NORM:
      fKeyPoints << S << "\n";
      break;
    case HAMMING:
      fKeyPoints << S * nOctaves << "\n";
      break;
  }

  fKeyPoints << M << "\n";  // Number of points
  D.convertTo(D, CV_32SC1);

  for (int j = 0; j < M; ++j) {
    fKeyPoints << KeyPointsLoc[j].pt.x << " " << KeyPointsLoc[j].pt.y << " "
               << KeyPointsLoc[j].octave << " " << KeyPointsLoc[j].angle << " "
               << KeyPointsLoc[j].size << " " << KeyPointsLoc[j].response
               << " ";

    if (dt == HAMMING) {
      for (int s = 0; s < S; ++s) {
        std::string binary = std::bitset<8>(D.at<int>(j, s)).to_string();
        fKeyPoints << binary[7] << " ";
        fKeyPoints << binary[6] << " ";
        fKeyPoints << binary[5] << " ";
        fKeyPoints << binary[4] << " ";
        fKeyPoints << binary[3] << " ";
        fKeyPoints << binary[2] << " ";
        fKeyPoints << binary[1] << " ";
        fKeyPoints << binary[0] << " ";
      }
    } else {
      for (int s = 0; s < S; s += 4) {
        fKeyPoints << D.at<int>(j, s) << " ";
        fKeyPoints << D.at<int>(j, s + 1) << " ";
        fKeyPoints << D.at<int>(j, s + 2) << " ";
        fKeyPoints << D.at<int>(j, s + 3) << " ";
      }
    }

    fKeyPoints << "\n";
  }

  fKeyPoints.close();
}