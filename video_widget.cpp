#include "video_widget.h"

VideoWidget::VideoWidget(QWidget *parent, const QString &inputUrl)
    : QWidget(parent),
    timer(new QTimer(this)),
    formatContext(nullptr),
    codecContext(nullptr),
    frame(av_frame_alloc()),
    packet(av_packet_alloc()),
    swsContext(nullptr),
    videoStreamIndex(-1),
    isPaused(false)
{
    av_log_set_level(AV_LOG_INFO);
    avformat_network_init();

    QByteArray byteArray = inputUrl.toLocal8Bit();
    const char *videoUrl = byteArray.data();

    AVDictionary *opts = nullptr;
    av_dict_set(&opts, "buffer_size", "90048576", 0); // 设置缓冲区大小为2MB

    if (avformat_open_input(&formatContext, videoUrl, nullptr, &opts) != 0) {
        qDebug() << "Could not open input";
        av_dict_free(&opts); // 确保在失败时释放字典
        return;
    }
    av_dict_free(&opts); // 确保成功时释放字典


    // 查找流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        qDebug() << "Could not find stream information";
        return;
    }

    // 查找视频流索引
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qDebug() << "Could not find video stream";
        return;
    }

    // 查找解码器
    AVCodecParameters *codecParams = formatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);

    if (!codec) {
        qDebug() << "Unsupported codec";
        return;
    }

    // 初始化解码器上下文
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        qDebug() << "Could not allocate video codec context";
        return;
    }

    if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
        qDebug() << "Could not copy codec parameters to context";
        return;
    }

    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        qDebug() << "Could not open codec";
        return;
    }

    // 初始化视频转换上下文
    swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                codecContext->width, codecContext->height, AV_PIX_FMT_BGR24,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);

    detectThread = new DetectThread(this);
    detectThread->start();

    connect(timer, &QTimer::timeout, this, &VideoWidget::onFrameDecoded);
    timer->start(33);
}

VideoWidget::~VideoWidget()
{
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    av_frame_free(&frame);
    av_packet_free(&packet);
    sws_freeContext(swsContext);
}

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if (!currentImage.isNull()) {
        QSize widgetSize = size(); // 获取窗口大小
        QImage scaledImage = currentImage.scaled(widgetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = (widgetSize.width() - scaledImage.width()) / 2;
        int y = (widgetSize.height() - scaledImage.height()) / 2;
        painter.drawImage(x, y, scaledImage);
    }
}

cv::Mat blobDetect(cv::dnn::Net model, cv::Mat blob){
    model.setInput(blob);
    return model.forward();
}

void VideoWidget::onFrameDecoded()
{
    if (isPaused) return;

    if (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecContext, packet) == 0) {
                if (avcodec_receive_frame(codecContext, frame) == 0) {
                    AVFrame *rgbFrame = av_frame_alloc();
                    rgbFrame->format = AV_PIX_FMT_RGB24;
                    rgbFrame->width = codecContext->width;
                    rgbFrame->height = codecContext->height;

                    if (av_frame_get_buffer(rgbFrame, 32) < 0) {
                        qDebug() << "Could not allocate frame buffer";
                        av_frame_free(&rgbFrame);
                        return;
                    }

                    sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, rgbFrame->data, rgbFrame->linesize);

                    {
                        // QMutexLocker locker(&mutex);
                        curMat = cv::Mat(codecContext->height, codecContext->width, CV_8UC3, rgbFrame->data[0], rgbFrame->linesize[0]).clone();
                    }

                    {
                        // QMutexLocker locker(&mutex);
                        for (const Box& box : curBoxes) {
                            cv::rectangle(curMat, box.rect, box.color, 2);
                            cv::putText(curMat, box.label, cv::Point(box.rect.x, box.rect.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, box.color, 2);
                        }
                    }

                    QImage img = matToQImage(curMat);
                    currentImage = img;

                    av_frame_free(&rgbFrame);

                    update();
                }
            }
        }
        av_packet_unref(packet);
    }
}

QImage VideoWidget::matToQImage(const cv::Mat &mat)
{
    if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    } else {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888).rgbSwapped();
    }
}
void VideoWidget::pauseVideo()
{
    isPaused = !isPaused; // 切换暂停状态
}
