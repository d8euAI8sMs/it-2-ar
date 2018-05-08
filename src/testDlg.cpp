
// testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "test.h"
#include "testDlg.h"
#include "afxdialogex.h"
#include "plot.h"

#include <iostream>
#include <array>
#include <numeric>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const double pi = 3.1415926535897932384626433832795;
const int    N  = 1024;

using points_ptr  = std::shared_ptr<plot::list_drawable::data_t>;

int         n                   = 256;
int         n1                  = 51;
int         n2                  = 136;
int         n1o                 = 0;
int         n2o                 = 0;
double      w0                  = 0.1;
double      w1                  = 0.2;
double      w2                  = 0.3;
double      d                   = 0;
points_ptr  data                = std::make_shared<points_ptr::element_type>(N);
points_ptr  e2                  = std::make_shared<points_ptr::element_type>(N);
points_ptr  smooth_e2           = std::make_shared<points_ptr::element_type>(N);
double      a1                  = 0;
double      a2                  = 0;
int         threshold           = -12;
int         filter_width_factor = 3;
int         filter_width        = (int) std::ceil(1 / w0) * filter_width_factor + ((int) std::ceil(1 / w0) * filter_width_factor) % 2;

plot::layer_drawable main_plot;
plot::layer_drawable e2_plot;
plot::layer_drawable smooth_e2_plot;
plot::world_t world(0, n, -1.1, 1.1);
plot::world_t e2_world;
plot::world_t smooth_e2_world;

void apply_x_scale()
{
    world.xmax = n;
    e2_world.xmax = n;
    smooth_e2_world.xmax = n;
}

double s(double x)
{
    double phi1 = (w0 - w1) * n1,
           phi2 = (w0 - w2) * n2;
    if (x < n1) return std::sin(w1 * x + phi1);
    else if (x > n2) return std::sin(w2 * x + phi2);
    else return std::sin(w0 * x);
}

double normal_distribution()
{
    double r = 0;
    for (int i = 0; i < 20; i++)
    {
        r += (double(rand()) / RAND_MAX) * 2 - 1;
    }
    return r / 20;
}

double r_ss(double diff)
{
    return pi * std::cos(diff * w0) / w0;
}

void calc_ar()
{
    double r0 = r_ss(0), r1 = r_ss(1), r2 = r_ss(2);
    a1 = (r0 * r1 - r1 * r2) / (r0 * r0 - r1 * r1);
    a2 = (r0 * r2 - r1 * r1) / (r0 * r0 - r1 * r1);
}

double ar(int i)
{
    return a1 * data->at(i - 1).y + a2 * data->at(i - 2).y;
}

double threshold_fn(double x)
{
    return std::pow(10, threshold);
}

CtestDlg::CtestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CtestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	main_plot.with(
		plot::plot_builder()
        .in_world(&world)
		.with_ticks(plot::palette::pen(RGB(150, 150, 0)))
		.with_x_ticks(0, 10, 0)
		.with_y_ticks(0, 5, 1)
        .with_function(&s)
        .with_data(plot::list_drawable::const_data_factory(data), plot::list_drawable::circle_painter(0, plot::palette::brush(RGB(255, 0, 0))), plot::palette::pen(RGB(255, 0, 0), 2))
		.build()
    );
	e2_plot.with(
		plot::plot_builder()
        .in_world(&e2_world)
		.with_ticks(plot::palette::pen(RGB(150, 150, 0)))
		.with_x_ticks(0, 10, 0)
        .with_y_ticks(0, 5, 4)
        .with_data(plot::list_drawable::const_data_factory(e2), plot::list_drawable::circle_painter(0, plot::palette::brush(RGB(0, 0, 255))), plot::palette::pen(RGB(0, 0, 255), 2))
		.build()
	);
    smooth_e2_plot.with(
        plot::plot_builder()
        .in_world(&smooth_e2_world)
        .with_ticks(plot::palette::pen(RGB(150, 150, 0)))
        .with_x_ticks(0, 10, 0)
        .with_y_ticks(0, 5, 4)
        .with_function(&threshold_fn, RGB(155, 0, 0))
        .with_data(plot::list_drawable::const_data_factory(smooth_e2), plot::list_drawable::circle_painter(0, plot::palette::brush(RGB(0, 255, 0))), plot::palette::pen(RGB(0, 155, 0), 2))
        .build()
    );
}

void CtestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_G, plot);
    DDX_Control(pDX, IDC_G2, plot_e2);
    DDX_Control(pDX, IDC_G3, plot_smooth_e2);
    DDX_Text(pDX, IDC_W0, w0);
    DDX_Text(pDX, IDC_W1, w1);
    DDX_Text(pDX, IDC_W2, w2);
    DDX_Text(pDX, IDC_N, n);
    DDX_Text(pDX, IDC_N1, n1);
    DDX_Text(pDX, IDC_N2, n2);
    DDX_Text(pDX, IDC_N1O, n1o);
    DDX_Text(pDX, IDC_N2O, n2o);
    DDX_Text(pDX, IDC_SNR, d);
    DDX_Text(pDX, IDC_THR, threshold);
    DDX_Text(pDX, IDC_FWF, filter_width_factor);
    DDX_Text(pDX, IDC_FW, filter_width);
    //DDX_Slider(pDX, IDC_SLIDER1, x_scale);
    //DDX_Control(pDX, IDC_SLIDER1, x_scale_slider);
}

BEGIN_MESSAGE_MAP(CtestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CtestDlg::OnBnClickedButton1)
    ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CtestDlg message handlers

BOOL CtestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    UpdateData(FALSE);

    //x_scale_slider.SetRange(1, x_scale * 10, TRUE);
    //x_scale_slider.SetPos(x_scale);

    UpdateData(TRUE);
    
    OnBnClickedButton1();

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void draw_layer(CWnd &wnd, plot::layer_drawable &ld)
{
    CDC &dc = *wnd.GetDC();
    SetBkMode(dc, TRANSPARENT);

    RECT bounds;
    wnd.GetClientRect(&bounds);
    plot::viewport vp({ bounds.left, bounds.right, bounds.top, bounds.bottom }, {});

    ld.draw(dc, vp);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CtestDlg::OnPaint()
{
	CDialog::OnPaint();

    draw_layer(plot, main_plot);
    draw_layer(plot_e2, e2_plot);
    draw_layer(plot_smooth_e2, smooth_e2_plot);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CtestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestDlg::OnBnClickedButton1()
{
    UpdateData(TRUE);

    apply_x_scale();

    data->resize(n);
    e2->resize(n);
    smooth_e2->resize(n);

    double *noise = new double[n];

    double data_energy = 0, noise_energy = 0;
    for (size_t i = 0; i < n; i++)
    {
        data->at(i) = { (double) i, s(i) };
        noise[i] = normal_distribution();
        data_energy += data->at(i).y * data->at(i).y;
        noise_energy += noise[i] * noise[i];
    }

    double alpha = std::sqrt(d * data_energy / noise_energy);
    
    for (size_t i = 0; i < n; i++)
    {
        data->at(i).y += alpha * noise[i];
    }

    delete[] noise;

    calc_ar();

    double e2max = 0;
    for (size_t i = 2; i < n; i++)
    {
        e2->at(i) = { (double) i, ((i < 2) ? 1 : data->at(i).y - ar(i)) };
        e2->at(i).y *= e2->at(i).y;
        e2max = max(e2max, e2->at(i).y);
    }

    filter_width = (int) std::ceil(1 / w0) * filter_width_factor;
    filter_width += filter_width % 2;

    double smoothe2max = 0;
    for (size_t i = filter_width / 2; i < n - filter_width / 2; i++)
    {
        double smooth = 0;
        for (int j = - filter_width / 2; j <= filter_width / 2; j++)
        {
            smooth += e2->at(i + j).y;
        }
        smooth /= filter_width;
        smooth_e2->at(i) = { (double) i, smooth };
        smoothe2max = max(smoothe2max, smooth_e2->at(i).y);
    }
    for (size_t i = 0; i < filter_width / 2; i++)
    {
        smooth_e2->at(i) = { (double) i, smooth_e2->at(filter_width / 2).y };
    }
    for (size_t i = n - filter_width / 2; i < n; i++)
    {
        smooth_e2->at(i) = { (double) i, smooth_e2->at(n - filter_width / 2 - 1).y };
    }

    e2_world.ymax = e2max * 1.1;
    e2_world.ymin = -e2max * 0.1;
    smooth_e2_world.ymax = smoothe2max * 1.1;
    smooth_e2_world.ymin = -smoothe2max * 0.1;

    double thr = std::pow(10, threshold);

    n1o = 0;
    n2o = n;
    for (size_t i = 0; i < n; i++)
    {
        if (smooth_e2->at(i).y < thr)
        {
            n1o = i - filter_width / 2;
            if (n1o <= 0) n1o += filter_width / 2;
            break;
        }
    }
    for (size_t i = n; i > 1; i--)
    {
        if (smooth_e2->at(i - 1).y < thr)
        {
            n2o = i - 1 + filter_width / 2;
            if (n2o >= n) n2o -= filter_width / 2;
            break;
        }
    }

    UpdateData(FALSE);

    Invalidate();
}

void CtestDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    UpdateData(TRUE);
    apply_x_scale();
    Invalidate();
}
