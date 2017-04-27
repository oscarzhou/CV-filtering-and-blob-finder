#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <set>


using namespace std;
using namespace cv;


#define Mpixel(image, x, y) ((uchar *)(((image).data)+(y)*((image).step)))[(x)]

#define MpixelB(image, x, y) ((uchar *)(((image).data)+(y)*((image).step)))[(x)*((image).channels())]
#define MpixelG(image, x, y) ((uchar *)(((image).data)+(y)*((image).step)))[(x)*((image).channels())+1]
#define MpixelR(image, x, y) ((uchar *)(((image).data)+(y)*((image).step)))[(x)*((image).channels())+2]

class CPoint
{
    public:

        int x;
        int y;

        CPoint();
        CPoint(int x, int y);
        virtual ~CPoint();

        bool operator< (const CPoint& pt) const;


    protected:
    private:
};

CPoint::CPoint()
{

}

CPoint::~CPoint()
{

}

CPoint::CPoint(int x, int y)
{
    this->x = x;
    this->y = y;
}

bool CPoint::operator< (const CPoint& pt) const
{
   if(this->x < pt.x)
    {
        return true;
    }
    else if(this->x == pt.x)
    {
        if(this->y < pt.y)
        {
            return true;
        }
        else if(this->y == pt.y)
        {
            return false;
        }
        else{
            return false;
        }
    }
    else
    {
        return false;
    }
};

struct RGB
{
    int r;
    int g;
    int b;
};

typedef std::set<CPoint> point_set;
typedef std::vector<point_set> set_vector;

/************************************************************************
*
* Function declaration
*
*************************************************************************/
Mat do_median_filter(Mat imgOri);
Mat do_threshold_filter(Mat imgOri, int flag);
Mat count_object(Mat imgOri, int* number);

int main(int argc, char** argv)
{

    clock_t  clockBegin, clockEnd;
    clockBegin = clock();
    if(argc!=2) {
        cout<<"needs 2 argument, e.g.image.jpg"<<endl;
        exit(0);
	}

    namedWindow("Step1: OriginalImage", 0);
    Mat imgOri = imread(argv[1], IMREAD_GRAYSCALE);
	imshow("Step1: OriginalImage", imgOri);

    namedWindow("Step2: MedianFilterImage", 0);
    Mat imgMedianFilter = do_median_filter(imgOri);
	imshow("Step2: MedianFilterImage", imgMedianFilter);

    namedWindow("Step3: ThresholdFilterImage", 0);
    Mat imgThresholdFilter = do_threshold_filter(imgMedianFilter, 1);
	imshow("Step3: ThresholdFilterImage", imgThresholdFilter);

    //namedWindow("Step4: SecondMedianFilterImage", 0);
    //Mat imgMedianFilter2 = do_median_filter(imgThresholdFilter);
	//imshow("Step4: SecondMedianFilterImage", imgMedianFilter2);
    int num = 0;



    namedWindow("Step5: ColoredImage", 0);
    Mat imgColored = count_object(imgThresholdFilter, &num);

    cout<<"the number is "<<num<<endl;
    char printit[100];
    sprintf(printit," %d",num);
    putText(imgColored, printit, cvPoint(10,30), FONT_HERSHEY_PLAIN, 2, cvScalar(255,255,255), 2, 8);

    imshow("Step5: ColoredImage", imgColored);



    clockEnd = clock();
    printf("the prgram runs %ld ms\n", clockEnd - clockBegin);
    waitKey();
    return 0;
}

/************************************************************************
*
* The function of sorting mask
* Parameter description:
* a: value1, b: value2
*
*************************************************************************/
inline void bubbleSort(int data[], int size)
{
    int temp;
    while(size > 1)
    {
        for(int i=0; i<size-1; i++)
        {
            if(data[i] > data[i + 1])
            {
                temp = data[i];
                data[i] = data[i + 1];
                data[i + 1] = temp;
            }
        }
        size--;
    }
}

/************************************************************************
*
* The impletation of median filter
* Parameter description:
* imgOri : input image with salt and pepper
*
*************************************************************************/
Mat do_median_filter(Mat imgOri)
{
    // Store the temperary data used for qsort function
    int pMask[9] = {0, 0, 0,
                    0, 0, 0,
                    0, 0, 0};
    Mat imgExtension;
    imgExtension.create(imgOri.rows + 2, imgOri.cols + 2, CV_8UC1);

    /************************************************************************
    *
    * Expand outer boundary for original image
    *
    *************************************************************************/
    for(int y=0; y<imgExtension.rows; y++)
        for(int x=0; x<imgExtension.cols; x++)
        {
            if(x==0)//left;
            {
                if(y==0)//left top
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x, y);
                }
                else if(y==imgExtension.rows-1) //left bottom
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x, y-2);
                }
                else // left except left top and left bottom
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x, y-1);
                }
            }
            else if(x==imgExtension.cols-1)//right
            {
                if(y==0)//right top
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x-2, y);
                }
                else if(y==imgExtension.rows-1) //right bottom
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x-2, y-2);
                }
                else // right except right top and right bottom
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x-2, y-1);
                }
            }
            else if(y==0)//top
            {
                if(x!=0 && x!=imgExtension.cols-1) //top except left top and right top
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x-1, y);
                }
            }
            else if(y==imgExtension.rows-1)//bottom
            {
                if(x!=0 && x!=imgExtension.cols-1) //bottom except left bottom and right bottom
                {
                    Mpixel(imgExtension, x, y) = Mpixel(imgOri, x-1, y-2);
                }
            }
            else
                Mpixel(imgExtension, x, y) = Mpixel(imgOri, x-1, y-1);

        }

    Mat imgPcd;
    imgPcd.create(imgOri.size(), CV_8UC1);
    /************************************************************************
    *
    * Calculating the median of each window then assigning new value into
    *
    *************************************************************************/
    for(int y=1; y<imgExtension.rows-1; y++)
        for(int x=1; x<imgExtension.cols-1; x++)
        {
            pMask[0] = Mpixel(imgExtension, x-1, y-1);
            pMask[1] = Mpixel(imgExtension, x, y-1);
            pMask[2] = Mpixel(imgExtension, x+1, y-1);
            pMask[3] = Mpixel(imgExtension, x-1, y);
            pMask[4] = Mpixel(imgExtension, x, y);
            pMask[5] = Mpixel(imgExtension, x+1, y);
            pMask[6] = Mpixel(imgExtension, x-1, y+1);
            pMask[7] = Mpixel(imgExtension, x, y+1);
            pMask[8] = Mpixel(imgExtension, x+1, y+1);

            bubbleSort(pMask, 9);

            // The important part. Improve the accuracy
            //if(pMask[4]<30)
            //    Mpixel(imgPcd, x-1, y-1) = 0;
            //else
                Mpixel(imgPcd, x-1, y-1) = pMask[4];
        }
    return imgPcd;
}

/************************************************************************
*
* The impletation of threshold filter
* Parameter description:
* imgOri : input image
* flag: 0, Otsu method  1, normal method
*
*************************************************************************/
Mat do_threshold_filter(Mat imgOri, int flag)
{
    Mat imgPcd;
    imgPcd.create(imgOri.size(), CV_8UC1);
    if(flag == 0)
    {
        threshold(imgOri, imgPcd, 0 ,255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    }
    else if(flag == 1)
    {
        threshold(imgOri, imgPcd, 50, 255, THRESH_BINARY); // threshold is set as 50 which is for increasing accuracy
    }
    return imgPcd;
}

/************************************************************************
*
* The impletation of blob algorithm
* Parameter description:
* imgOri : input image with salt and pepper
*
*************************************************************************/
Mat count_object(Mat imgOri, int* number)
{
    // Store all the sets which stand for the object
    set_vector vec;
    int counter = -1, s1, s2;

    int width = imgOri.cols;
    int height = imgOri.rows;

    std::vector<int> matrixA;
    matrixA.assign(width*height, -1);

    /************************************************************************
    *
    * The implementation of object labeling algorithm using 4-adjacency
    *
    *************************************************************************/
    for(int y=1; y<height; y++)
    {
        for(int x=1; x<width; x++)
        {
            if((int)Mpixel(imgOri, x, y)  != 0)
            {
                if((int)Mpixel(imgOri, x-1, y) != 0 || (int)Mpixel(imgOri, x, y-1) != 0)
                {
                    s1 = matrixA[(x - 1) + (y * width)];
                    s2 = matrixA[x +  ((y - 1) * width)];


                    if(s1 != -1)
                    {
                        point_set* pset = &vec[s1];
                        pset->insert(CPoint(x, y));
                        matrixA[x + y * width] = s1;
                    }

                    if(s2 != -1)
                    {
                        point_set* pset = &vec[s2];
                        pset->insert(CPoint(x, y));
                        matrixA[x + y * width] = s2;
                    }

                    if((s1 != s2) && (s1 != -1) && (s2 != -1))
                    {
                        point_set* pset2 = &vec[s2];
                        point_set* pset1 = &vec[s1];
                        for(point_set::iterator it=pset2->begin(); it!=pset2->end(); it++)
                        {
                            matrixA[((CPoint)*it).x + ((CPoint)*it).y * width] = s1;
                        }

                        pset1->insert(pset2->begin(), pset2->end());
                        pset2->clear();
                        pset2 = NULL;
                    }
                }
                else
                {
                    counter++;
                    point_set setofobj;

                    setofobj.insert(CPoint(x, y));
                    vec.push_back(setofobj);
                    matrixA[x + y * width] = counter;;
                }
            }

        }
    }

    /************************************************************************
    *
    * count the number of valid set, that is, the number of objects and color objects
    *
    *************************************************************************/
    int num = 0;
    if(!vec.empty())
    {
        for(set_vector::iterator it=vec.begin(); it!=vec.end(); it++)
        {
            point_set poset = *it;
            if(!poset.empty() && poset.size()>20) // size>20 is for increasing the accuracy
            {
                num++;
            }
        }
    }
    *number = num;

    std::vector<RGB> pRGB;
    for(int i=0; i<counter; i++)
    {
        RGB srgb;
        srgb.r = rand()%255;
        srgb.g = rand()%255;
        srgb.b = rand()%255;
        pRGB.push_back(srgb);
    }

    Mat imgColored;
    imgColored.create(imgOri.size(), CV_8UC3);
    for(int y=1; y<height; y++)
    {
        for(int x=1; x<width; x++)
        {
            int index = matrixA[x + y * width];
            if(index == -1)
            {
                MpixelR(imgColored, x, y) = Mpixel(imgOri, x, y);
                MpixelG(imgColored, x, y) = Mpixel(imgOri, x, y);
                MpixelB(imgColored, x, y) = Mpixel(imgOri, x, y);
            }
            else
            {
                MpixelR(imgColored, x, y) = pRGB[index].r;
                MpixelG(imgColored, x, y) = pRGB[index].g;
                MpixelB(imgColored, x, y) = pRGB[index].b;
            }
        }
    }
    return imgColored;

}
