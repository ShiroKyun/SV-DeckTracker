#ifndef FRMWINDOW_H
#define FRMWINDOW_H

#include <QMainWindow>
#include "perceptualhash.h"
#include "cardlist.h"
#include "svdatabase.h"
#include <QDir>
#include <vector>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

namespace Ui {
class frmWindow;
enum class STATE{MYTURN, FINDCARD};
}

class frmWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void update();

public:
    explicit frmWindow(QWidget *parent = 0);
    ~frmWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::frmWindow *ui;

    cv::Mat matTexture;
    ulong64 matTexturePhash;


    cv::Mat mat;

    HDC hdcScreen;
    HDC hdc;
    HBITMAP hbmp;
    HWND handle;


    // bounding box for window
    int height;
    int width;
    int left;
    int top;

    // bounding box for "MY TURN"
    int boxHeight;
    int boxWidth;
    int boxLeft;
    int boxTop;

    // bounding box fopr FIND CARD
    int cardHeight;
    int cardWidth;
    int cardLeft;
    int cardTop;

    QDir dir;

    Ui::STATE curState;

    // This keeps track of current deck in game; This has count. Different from the ones in files.
    //cardlist currentDeck;

    // This is to hold all the info about the deck in the game
    //cardlist playingDeck;

    // Database of all the cards in the game
    //svDatabase cardDatabase;

    //debug stuff
    cv::Mat resultMat;
    int counter;
    int turncounter;
    int ignoreNext;
    int toyou;

    std::vector<int> id;
    std::vector<int> count;

    void savedata();
};

#endif // FRMWINDOW_H
