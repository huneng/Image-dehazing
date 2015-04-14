#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <assert.h>

#define HU_MIN(a, b) ((a) < (b) ? (a) : (b))
#define HU_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HU_ABS(a) ((a) < 0 ? (-a) : (a))

#define OMEGA 0.9375
#define T0 0.25
#define WIN_SIZE 1

#define SHOW_IMAGE(x) {cv::imshow(#x, x); cv::waitKey();}

cv::Mat& get_dark_channels(cv::Mat &src, cv::Mat &darkChannel, int ws)
{
    int rows = src.rows;
    int cols = src.cols;

    cv::Mat darkChan(rows, cols, CV_8UC1, cv::Scalar(255));
    darkChannel = cv::Mat(rows, cols, CV_8UC1, cv::Scalar(255));

    for(int y = 0; y < rows; y++)
    {
        for(int x = 0; x < cols; x++)
        {
            cv::Vec3b vec = src.at<cv::Vec3b>(y, x);
            darkChan.at<uchar>(y, x) = HU_MIN( HU_MIN(vec[0], vec[1]), vec[2] );
        }
    }


    for(int y = 0; y < rows; y++)
    {
        int y0 = HU_MAX(0, y-ws);
        int y1 = HU_MIN(y+ws, rows-1);

        for(int x = 0; x < cols; x++)
        {
            int x0 = HU_MAX(0, x-ws);
            int x1 = HU_MIN(x+ws, cols-1);

            uchar minValue = 255;

            for(int j = y0; j <= y1; j++)
                for(int i = x0; i <= x1; i++)
                    minValue = HU_MIN(minValue, darkChan.at<uchar>(j, i));

            darkChannel.at<uchar>(y, x) = minValue;
        }
    }
}


int get_airlight(cv::Mat &darkChannel)
{
    int rows = darkChannel.rows;
    int cols = darkChannel.cols;
    int step = darkChannel.step;

    uchar* data = darkChannel.data;

    int maxValue = 0;

    for(int y = 0; y < rows; y++)
    {
        for(int x = 0; x < cols; x++)
            maxValue = HU_MAX(maxValue, data[x]);
        data += step;
    }

    return maxValue;
}


void filter_image(cv::Mat &src, cv::Mat &dst)
{
    const int thresh = 80;

    int rows = src.rows;
    int cols = src.cols;
    int step = src.step;
    int chan = src.channels();

    assert(chan == 3);

    dst = src.clone();

    for(int y = 1; y < rows-1; y ++)
    {
        int y0 = y-1;
        int y1 = y+1;

        for(int x = 1; x < cols-1; x++)
        {
            int x0 = x-1;
            int x1 = x+1;

            for(int c = 0; c < chan; c++)
            {
                int lt = src.at<cv::Vec3b>(y0, x0)[c];
                int ct = src.at<cv::Vec3b>(y0, x)[c];
                int rt = src.at<cv::Vec3b>(y0, x1)[c];
                int lc = src.at<cv::Vec3b>(y, x0)[c];
                int self = src.at<cv::Vec3b>(y, x)[c];
                int rc = src.at<cv::Vec3b>(y, x1)[c];
                int lb = src.at<cv::Vec3b>(y1, x0)[c];
                int cb = src.at<cv::Vec3b>(y1, x)[c];
                int rb = src.at<cv::Vec3b>(y1, x1)[c];

                if(HU_ABS(rb - lt) >= thresh || HU_ABS(lb - rt) >= thresh)
                {
                    int value = self * 4 + (lt + rt + lb + rb) * 2 +
                        (ct + lc + rc + cb);

                    dst.at<cv::Vec3b>(y, x)[c] = uchar(value / 16);
                }
                else if(HU_ABS(ct - cb) >= thresh || HU_ABS(lc - rc) >= thresh)
                {
                    int value = self * 4 + (ct + cb + lc + rc) * 2 +
                        (lt + rt + lb + rb);

                    dst.at<cv::Vec3b>(y, x)[c] = uchar(value / 16);
                }
                else{
                    int value = self + (lt + rt + lb + rb)  +
                        (ct + lc + rc + cb);
                    dst.at<cv::Vec3b>(y, x)[c] = uchar(value / 9);
                }
            }
        }
    }
}


void estimate_transmission_ori(cv::Mat &darkChannel, int airLight, cv::Mat &trans, float omega)
{
    int rows = darkChannel.rows;
    int cols = darkChannel.cols;
    int step = darkChannel.step;

    uchar *data = darkChannel.data;

    trans = cv::Mat(rows, cols, CV_32FC1, cv::Scalar(0));

    for(int y = 0; y < rows; y++)
    {
        for(int x = 0; x < cols; x++)
        {
            float value = data[x];

            trans.at<float>(y, x) = 1 - omega * value / airLight;
        }

        data += step;
    }
}


void estimate_transmission(cv::Mat &img, int airLight, cv::Mat &trans, float omega)
{
    int rows = img.rows;
    int cols = img.cols;

    assert(img.channels() == 3 && img.type() == CV_8UC3);

    trans = cv::Mat(rows, cols, CV_32FC1, cv::Scalar(0));


    for(int y = 0; y < rows; y++)
    {
        for(int x = 0; x < cols; x++)
        {
            cv::Vec3b vec = img.at<cv::Vec3b>(y, x);

            float value = HU_MIN( vec[0], HU_MIN(vec[1], vec[2]) );

            trans.at<float>(y, x) = 1 - omega * value / airLight;
        }
    }
}


void dehaze_image(cv::Mat &img, cv::Mat &trans, int airLight, float t0, cv::Mat &res)
{
    int rows = img.rows;
    int cols = img.cols;

    assert(img.rows == trans.rows && img.cols == trans.cols);
    assert(img.channels() == 3);

    res = cv::Mat(rows, cols, CV_8UC3, cv::Scalar(0, 0, 0));

    uchar *srcData = img.data;
    uchar *resData = res.data;

    for(int y = 0; y < rows; y++)
    {
        for(int x = 0; x < cols; x++)
        {
            float t = HU_MAX(trans.at<float>(y, x), t0);

            int idx = 3 * x;

            for(int c = 0; c < 3; c++)
            {
                float value = (srcData[idx + c] - airLight)/t + airLight;

                if( value < 0)
                    resData[idx + c] = 0;
                else if( value > 255)
                    resData[idx + c] = 255;
                else
                    resData[idx + c] = (uchar)(value);
            }
        }

        srcData += img.step;
        resData += res.step;
    }
}


void merge_image(cv::Mat &img1, cv::Mat &img2, cv::Mat &res);


void test_origin_method(cv::Mat &src, cv::Mat &dst)
{
    cv::Mat darkChannel, trans, dehazedImg;

    int airLight;

    get_dark_channels(src, darkChannel, WIN_SIZE);

    airLight = get_airlight(darkChannel);

    estimate_transmission_ori(darkChannel, airLight, trans, OMEGA);

    dehaze_image(src, trans, airLight, T0, dehazedImg);

    merge_image(src, dehazedImg, dst);
}


void test_new_method(cv::Mat &src, cv::Mat &dst)
{
    cv::Mat darkChannel, trans, dehazedImg, blur;

    int airLight;

    get_dark_channels(src, darkChannel, WIN_SIZE);

    airLight = get_airlight(darkChannel);

//    filter_image(src, blur);
//    estimate_transmission(blur, airLight, trans, OMEGA);

    estimate_transmission(src, airLight, trans, OMEGA);

    dehaze_image(src, trans, airLight, T0, dehazedImg);

    merge_image(src, dehazedImg, dst);

}


int main(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage:%s [input image] [output image]\n", argv[0]);
        return 1;
    }

    cv::Mat img, res;

    img = cv::imread(argv[1]);

    if(img.empty())
    {
        printf("Can't open image %s\n", argv[1]);
        return 1;
    }

    assert(img.channels() == 3);

    test_new_method(img, res);
    //test_origin_method(img, res);


    if(!cv::imwrite(argv[2], res))
    {
        printf("Can't write image %s\n", argv[2]);
        return 1;
    }

    return 0;
}


void merge_image(cv::Mat &img1, cv::Mat &img2, cv::Mat &res)
{
    int rows1 = img1.rows;
    int cols1 = img1.cols;
    int rows2 = img2.rows;
    int cols2 = img2.cols;

    assert(img1.type() == img2.type());

    res = cv::Mat(HU_MAX(rows1, rows2), cols1 + cols2, img1.type(), cv::Scalar::all(0));

    cv::Mat left(res, cv::Rect(0, 0, cols1, rows1));
    cv::Mat right(res, cv::Rect(cols1, 0, cols2, rows2));

    left += img1;
    right += img2;
}
