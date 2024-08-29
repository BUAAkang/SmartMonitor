#include "detect_thread.h"
#include "video_widget.h"

cv::dnn::Net readNetFromQrc(const QString &qrcPath) {
    QFile file(qrcPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open ONNX file:" << qrcPath;
        return cv::dnn::Net();
    }

    QByteArray fileData = file.readAll();
    file.close();

    std::vector<uchar> buffer(fileData.begin(), fileData.end());
    cv::Mat rawData(1, buffer.size(), CV_8UC1, buffer.data());

    return cv::dnn::readNetFromONNX(rawData);
}

DetectThread::DetectThread(VideoWidget *mainWidget):
    mainWidget(mainWidget)
{
    yoloNet = readNetFromQrc(":/yolov8n.onnx");
    lpNet = readNetFromQrc(":/LP-detection.onnx");

    if (yoloNet.empty() || lpNet.empty()) {
        qDebug() << "Failed to load model!";
        return;
    }
}

void DetectThread::run() {
    cv::Mat image;
    while (true) {
        {
            // QMutexLocker locker(&mutex);
            if (mainWidget->curMat.empty()) {
                QThread::msleep(33);
                continue;
            }
            image = mainWidget->curMat;
        }
        imageDetect(image);
    }
}

void DetectThread::imageDetect(cv::Mat image){
    try{
        cv::Mat blob;
        blob = cv::dnn::blobFromImage(image, 1.0/255, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);
        yoloNet.setInput(blob);
        cv::Mat yoloDetections = yoloNet.forward();
        lpNet.setInput(blob);
        cv::Mat lpDetections = lpNet.forward();

        double image_width = image.cols;
        double image_height = image.rows;
        double scaled_width = 640;
        double scaled_height = 640;

        int index[3] = {4, 6, 4}; // human, car, license
        float conf[3] = {0.6, 0.3, 0.3};

        std::vector<cv::Rect> rects;
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<Box> boxes;

        for (int i = 0; i < 8400; ++i) {
            int mode = -1;

            if (yoloDetections.at<float>(0, 4, i) > conf[0])
                mode = 0;
            else if (yoloDetections.at<float>(0, 6, i) > conf[1])
                mode = 1;
            else
                continue;

            double centerX = yoloDetections.at<float>(0, 0, i) * (image_width / scaled_width);
            double centerY = yoloDetections.at<float>(0, 1, i) * (image_height / scaled_height);
            double width = yoloDetections.at<float>(0, 2, i) * (image_width / scaled_width);
            double height = yoloDetections.at<float>(0, 3, i) * (image_height / scaled_height);

            double left = centerX - width / 2;
            double top = centerY - height / 2;
            cv::Rect rect(left, top, width, height);

            rects.push_back(rect);
            classIds.push_back(mode);
            confidences.push_back(yoloDetections.at<float>(0, index[mode], i));
        }

        for (int i = 0; i < 8400; ++i) {
            if (lpDetections.at<float>(0, 4, i) < conf[2])
                continue;

            double centerX = lpDetections.at<float>(0, 0, i) * (image_width / scaled_width);
            double centerY = lpDetections.at<float>(0, 1, i) * (image_height / scaled_height);
            double width = lpDetections.at<float>(0, 2, i) * (image_width / scaled_width);
            double height = lpDetections.at<float>(0, 3, i) * (image_height / scaled_height);

            double left = centerX - width / 2;
            double top = centerY - height / 2;
            cv::Rect rect(left, top, width, height);

            rects.push_back(rect);
            classIds.push_back(2);
            confidences.push_back(lpDetections.at<float>(0, index[2], i));
        }

        std::vector<int> indices;
        float nmsThreshold = 0.6;
        cv::dnn::NMSBoxes(rects, confidences, 0.0, nmsThreshold, indices);

        // std::cout << "raw size: " << rects.size() << " NMS size: " << indices.size() << std::endl;

        for (size_t i = 0; i < indices.size(); ++i) {
            int idx = indices[i];
            cv::Rect rect = rects[idx];
            int mode = classIds[idx];
            std::string label;
            cv::Scalar color;
            std::ostringstream ss;

            switch (mode) {
            case 0:
                ss << "Man " << std::fixed << std::setprecision(2) << confidences[idx];
                color = cv::Scalar(0, 0, 255);
                break;
            case 1:
                ss << "Car " << std::fixed << std::setprecision(2) << confidences[idx];
                color = cv::Scalar(0, 255, 0);
                break;
            default:
                ss << "License " << std::fixed << std::setprecision(2) << confidences[idx];
                color = cv::Scalar(255, 0, 0);
                break;
            }
            label = ss.str();
            Box box = {color, label, rect};
            boxes.push_back(box);
            blob.release();
            yoloDetections.release();
            lpDetections.release();
        }
        {
            // QMutexLocker locker(&mutex);
            mainWidget->curBoxes = boxes;
        }
    } catch (const cv::Exception& e) {  // 捕获OpenCV异常
        std::cerr << "OpenCV error: " << e.what() << std::endl;
    } catch (const std::exception& e) {  // 捕获标准库异常
        std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {  // 捕获所有其他类型的异常
        std::cerr << "Unknown error occurred!" << std::endl;
    }
}
