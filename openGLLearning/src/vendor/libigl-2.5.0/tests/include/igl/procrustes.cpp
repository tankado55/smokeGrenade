#include <test_common.h>
#include <igl/procrustes.h>

TEST_CASE("procrustes", "[igl]")
{
    using namespace igl;
    Eigen::Matrix<double, Eigen::Dynamic, 3> X(5, 3);
    X <<
        -0.7, -1.19, 0.8,
        0.86, 0.57, -0.19,
        -0.44, 0.65, -0.14,
        -1.67, 2.75, 0.65,
        2.67, -0.41, -1.34;
    Eigen::Matrix<double, Eigen::Dynamic, 3> Y(5, 3);
    Y <<
        -0.32, -0.32, -0.11,
        0.54, -0.29, -0.2,
        -0.59, -0.9, -0.45,
        0.51, 1.18, 0.33,
        -0.19, -0.82, 0.24;

    Eigen::Matrix<double, Eigen::Dynamic, 3> Xref(5, 3);
    Eigen::Matrix<double, Eigen::Dynamic, 3> Xhat;
    double scale;
    Eigen::Matrix3d R;
    Eigen::Vector3d t;

    const float eps = 1e-6;

    procrustes(X, Y, /*includeScaling*/ true, /*includeReflections*/ false, scale, R, t);
    Xhat = (scale * X * R).rowwise() + t.transpose();
    Xref <<
        -5.897354483604431152e-01, -1.769234091043472290e-01, -2.173528671264648438e-01,
        7.268430292606353760e-02, -3.918691575527191162e-01, 8.458440750837326050e-02,
        2.018260955810546875e-02, -8.983440697193145752e-02, -1.527965068817138672e-01,
        3.137306272983551025e-01, 5.987602472305297852e-01, -1.497855782508850098e-02,
        1.331380605697631836e-01, -1.090133190155029297e+00, 1.105435565114021301e-01;
    test_common::assert_near(Xhat, Xref, eps);

    procrustes(X, Y, /*includeScaling*/ false, /*includeReflections*/ false, scale, R, t);
    Xhat = (scale * X * R).rowwise() + t.transpose();
    Xref <<
        -1.958898901939392090e+00, -5.157226324081420898e-02, -6.409312486648559570e-01,
        2.679601013660430908e-01, -7.741562128067016602e-01, 3.740924298763275146e-01,
        9.146496653556823730e-02, 2.411951720714569092e-01, -4.239118695259094238e-01,
        1.078286409378051758e+00, 2.556046247482299805e+00, 3.939124941825866699e-02,
        4.711876809597015381e-01, -3.121513128280639648e+00, 4.613594114780426025e-01;
    test_common::assert_near(Xhat, Xref, eps);

    procrustes(X, Y, /*includeScaling*/ true, /*includeReflections*/ true, scale, R, t);
    Xhat = (scale * X * R).rowwise() + t.transpose();
    Xref <<
        -4.254432320594787598e-01, -2.139694541692733765e-01, -4.958550631999969482e-01,
        1.176588535308837891e-01, -4.038954973220825195e-01, 1.682620495557785034e-02,
        -8.266170322895050049e-02, -6.510947644710540771e-02, 9.997285902500152588e-03,
        2.587105333805084229e-01, 6.245076656341552734e-01, 8.087203651666641235e-02,
        8.173567056655883789e-02, -1.091533184051513672e+00, 1.981595158576965332e-01;
    test_common::assert_near(Xhat, Xref, eps);

    procrustes(X, Y, /*includeScaling*/ false, /*includeReflections*/ true, scale, R, t);
    Xhat = (scale * X * R).rowwise() + t.transpose();
    Xref <<
        -1.385619997978210449e+00, -1.769195646047592163e-01, -1.554054498672485352e+00,
        4.127053320407867432e-01, -8.058047294616699219e-01, 1.435410976409912109e-01,
        -2.505982220172882080e-01, 3.159871101379394531e-01, 1.209291219711303711e-01,
        8.797571659088134766e-01, 2.599454402923583984e+00, 3.556103110313415527e-01,
        2.937560975551605225e-01, -3.082717418670654297e+00, 7.439738512039184570e-01;
    test_common::assert_near(Xhat, Xref, eps);
}
