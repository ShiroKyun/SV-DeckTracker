#include "frmwindow.h"
#include "ui_frmwindow.h"
#include "asmopencv.h"
#include "perceptualhash.h"
#include "cardlist.h"

#include <QtWinExtras/QtWin>
#include <QPixmap>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <qtimer.h>
#include <Windows.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat=0);

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

std::wstring s2ws(const std::string& s);

frmWindow::frmWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmWindow),
    dir("."),
    currentDeck("current", &cardDatabase),
    playingDeck("Don't use me", &cardDatabase)
{
    ui->setupUi(this);

    // verify bluestacks is open and set up a window handle to the screen
    handle = 0;
    mat = 0;
    matTexture = 0;
    cardFound = false;

    ///DEBUG
    /// ADD IN SOME SAMPLE CARDS FOR CURRENT DECK
    ///IDEALLY WE LOAD IN PLAYINGDECK, THEN COPY INTO CURRENT
    playingDeck.addCard(100211010);
    playingDeck.addCard(100211010);
    playingDeck.addCard(100211010);
    playingDeck.addCard(101232020);
    playingDeck.addCard(101211020);
    playingDeck.addCard(101211020);
    playingDeck.addCard(101211020);
    playingDeck.addCard(101211110);
    playingDeck.addCard(101211110);
    playingDeck.addCard(101221010);
    playingDeck.addCard(101221010);
    playingDeck.addCard(101221010);
    playingDeck.addCard(100214010);
    playingDeck.addCard(100211030);
    playingDeck.addCard(100211030);
    playingDeck.addCard(100211030);
    playingDeck.addCard(101211060);
    playingDeck.addCard(101211060);
    playingDeck.addCard(101211090);
    playingDeck.addCard(101211090);
    playingDeck.addCard(101211090);
    playingDeck.addCard(101221070);
    playingDeck.addCard(100211040);
    playingDeck.addCard(100211040);
    playingDeck.addCard(100221010);
    playingDeck.addCard(100221010);
    playingDeck.addCard(100221010);
    playingDeck.addCard(101211070);
    playingDeck.addCard(101211070);
    playingDeck.addCard(101221100);
    playingDeck.addCard(101031020);
    playingDeck.addCard(101031020);
    playingDeck.addCard(101024030);
    playingDeck.addCard(101241020);
    playingDeck.addCard(101241020);
    playingDeck.addCard(100221020);
    playingDeck.addCard(100221020);
    playingDeck.addCard(101241030);
    playingDeck.addCard(101234020);
    playingDeck.addCard(101231040);

    QListView *list =  ui->listView;
    model = new QStandardItemModel();
    CardDelegate * delegate = new CardDelegate(&cardDatabase);

    list->setItemDelegate(delegate);
    list->setModel(model);
    currentDeck = playingDeck;
    loadDeck(model);
    list->show();

    QFile file(dir.absolutePath() + "/settings.ini");
    if (!file.open(QIODevice::ReadOnly))
        qWarning("Couldn't find settings.ini");

    QTextStream in(&file);

    QMap<QString, QString> settingsMap;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList splitLines = line.split("=");
        settingsMap.insert(splitLines[0],splitLines[1]);
    }
    file.close();

    std::string appName = settingsMap.value("Windowname").toStdString();
    std::wstring stemp = s2ws(appName);
    LPCWSTR result = stemp.c_str();

    handle = ::FindWindow(NULL, result);

    //load values here
    int topborder = settingsMap.value("Topborder").toInt();
    int botborder = settingsMap.value("Botborder").toInt();
    int leftborder = settingsMap.value("Leftborder").toInt();
    int rightborder = settingsMap.value("Rightborder").toInt();

    //Window rect
    RECT rc;
    GetClientRect(handle, &rc);
    width = (rc.right - rc.left) - rightborder - leftborder;
    height = (rc.bottom - rc.top) - topborder - botborder;
    top = rc.top+topborder;
    left = rc.left+leftborder;

    boxLeft = (int)round(0.261 * width) + left;
    boxTop = (int)round(0.341 * height) + top;
    boxWidth = (int)round(0.4735 * width);
    boxHeight = (int)round(0.1965 * height);

    theirLeft = (int)round(0.1636 * width) + left;
    theirTop = (int)round(0.3285 * height) + top;
    theirWidth = (int)round(0.6646 * width);
    theirHeight = (int)round(0.2206 * height);

    costLeft = (int)round(0.5795 * width) + left;
    costTop = (int)round(0.4461 * height) + top;
    costWidth = (int)round(0.02789 * width);
    costHeight = (int)round(0.04956 * height);



    //load all costs here into a vector of phashes
    for (int i = 1; i < 11; i++)
    {
        QString filename = dir.absolutePath() + "/CostGame/" + QString::number(i)+ ".png";
        cv::Mat numbermap;
        numbermap = cv::imread(filename.toStdString());
        ulong64 myhash = PerceptualHash::phash(numbermap);
        numberPHash.push_back(myhash);
    }
    QString filename = dir.absolutePath() + "/CostGame/18.png";
    cv::Mat numbermap;
    numbermap = cv::imread(filename.toStdString());
    ulong64 myhash = PerceptualHash::phash(numbermap);
    numberPHash.push_back(myhash);


    if (handle != 0)
    {
        // Start the update loop to check for cards in images
        counter = 0;
        turncounter = 0;
        handleValid = true;

        //create bitmap and screen to save rect
        hdcScreen = GetDC(NULL);
        hdc = CreateCompatibleDC(hdcScreen);
        hbmp = CreateCompatibleBitmap(hdcScreen,
            width + leftborder, height + topborder);
        SelectObject(hdc, hbmp);

        //Guess the current state of the image
        curState = Ui::STATE::MYTURN;

        //Create a "you start" mat
        matTexture = cv::imread("pic.png");
        matTexturePhash = PerceptualHash::phash(matTexture);
        matTexture = cv::imread("pic2.png");
        theirTexturePhash = PerceptualHash::phash(matTexture);

        ignoreNext = 0;
        passed = false;
        turnDraw =0;

        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(update()));
        timer->start(100);

    }
}

frmWindow::~frmWindow()
{
    delete ui;
}

void frmWindow::sortDeck()
{
    for (int i = 1; i < currentDeck.cardsInDeck.size(); i++)
    {
        int j = i;

        while (j > 0 &&
               cardDatabase.getCard(currentDeck.cardsInDeck[j-1]).manaCost >
               cardDatabase.getCard(currentDeck.cardsInDeck[j]).manaCost)
        {
            int tempCard = currentDeck.cardsInDeck[j];
            int tempCount = currentDeck.countInDeck[j];
            ulong64 tempHash = currentDeck.deckPHash[j];
            currentDeck.cardsInDeck[j] = currentDeck.cardsInDeck[j-1];
            currentDeck.countInDeck[j] = currentDeck.countInDeck[j-1];
            currentDeck.deckPHash[j] = currentDeck.deckPHash[j-1];
            currentDeck.cardsInDeck[j-1] = tempCard;
            currentDeck.countInDeck[j-1] = tempCount;
            currentDeck.deckPHash[j-1] = tempHash;
            j--;
        }
    }
}

void frmWindow::loadDeck(QStandardItemModel* model)
{
    //clear model first
    model->clear();

    //sort the current deck before loading it into model
    sortDeck();

    //Push all cards from current deck into model
    for (int i = 0; i < currentDeck.cardsInDeck.size(); i++)
    {
        Card card = cardDatabase.getCard( currentDeck.cardsInDeck[i]);
        QStandardItem *item = new QStandardItem();
        item->setData(card.manaCost,CardDelegate::Cost);
        item->setData(card.ID,CardDelegate::ID);
        item->setData(card.name,CardDelegate::Name);
        item->setData(currentDeck.countInDeck[i],CardDelegate::Amount);
        item->setEditable(false);
        model->appendRow(item);
    }
}

void frmWindow::update()
{
    /// In the update function, we will continous loop to try
    /// and guess what state the program is in. Mainly want to
    /// see if a game had started or not.
    //decrement all if exist
    if (ignoreNext > 0)ignoreNext--;


    //otherwise check for card
    if (handleValid && ::IsWindow(handle))
    {
        PrintWindow(handle, hdc, PW_CLIENTONLY);

        QPixmap pixmap = qt_pixmapFromWinHBITMAP(hbmp);

        QRect boxRect;
        QPixmap drawer;
        ulong64 imagePHash;
        ulong64 theirPHash;


        int distance;

        switch (curState)
        {
        case Ui::STATE::MYTURN:
            //Print to memory hdc

            boxRect.setRect(boxLeft,boxTop,boxWidth,boxHeight);
            drawer = pixmap.copy(boxRect);
            mat = ASM::QPixmapToCvMat(drawer);
            imagePHash = PerceptualHash::phash(mat);

            distance = PerceptualHash::hammingDistance(matTexturePhash, imagePHash);


            if (distance < 20 && ignoreNext < 1)
            {
                setWindowTitle("Now it's my turn!");
                ignoreNext = 50;
                turncounter++;
                QString trn = QString::number(turncounter);
                ui->pushButton->setText(trn);
                curState = Ui::STATE::FINDCARD;
                cardFound = false;
            }

            break;
        case Ui::STATE::FINDCARD:

            //Get the card cost (hopefully)
            //Try and find what the cost is?
            boxRect.setRect(costLeft,costTop,costWidth,costHeight);
            drawer = pixmap.copy(boxRect);
            mat = ASM::QPixmapToCvMat(drawer);
            imagePHash = PerceptualHash::phash(mat);

            PerceptualHash::ComparisonResult result = PerceptualHash::best(imagePHash, numberPHash);

            int cost = result.index+1;
            int costDistance = result.distance;

            //Perspective shift on card for better readability (why are they tilted)
            mat = ASM::QPixmapToCvMat(pixmap);
            cv::Point2f input[4];
            cv::Point2f output[4];

            //in case of window size changed, use percentages // ADD SHIFT
            input[0] = cv::Point2f(0.6047f * width + left,0.8208f * height + top);
            input[1] = cv::Point2f(0.5968f * width + left,0.4958f * height + top);
            input[2] = cv::Point2f(0.7359f * width + left,0.4958f * height + top);
            input[3] = cv::Point2f(0.7492f * width + left,0.8194f * height + top);

            //doesnt matter here, so long as output size fits here
            output[0] = cv::Point2f(0,200);
            output[1] = cv::Point2f(0,0);
            output[2] = cv::Point2f(133,0);
            output[3] = cv::Point2f(133,200);

            cv::Mat lambda( 2, 4, CV_32FC1 );
            lambda = cv::Mat::zeros( mat.rows, mat.cols, mat.type() );

            lambda = cv::getPerspectiveTransform(input,output);
            cv::warpPerspective(mat, resultMat, lambda, cv::Size(133,200));

            imagePHash = PerceptualHash::phash(resultMat);

            PerceptualHash::ComparisonResult bestguess = PerceptualHash::best(imagePHash, currentDeck.deckPHash);

            if (bestguess.distance < 15 || costDistance < 15)
            {
                ui->tableWidget->clearContents();

                ui->tableWidget->setItem(0,0,new QTableWidgetItem(cardDatabase.getCard(currentDeck.cardsInDeck[bestguess.index]).name));
                ui->tableWidget->setItem(0,1,new QTableWidgetItem(QString::number(bestguess.distance)));
                QString heh = "Cost is " + QString::number(cost);
                ui->tableWidget->setItem(1,0,new QTableWidgetItem(heh));
                ui->tableWidget->setItem(1,1,new QTableWidgetItem(QString::number(costDistance)));
            }

            break;
        }
    }
    else
    {
        handleValid = false;
        setWindowTitle("Window is closed");
    }
}

void frmWindow::on_pushButton_clicked()
{
    cv::imwrite("filetitledraw.png", resultMat);
}

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
