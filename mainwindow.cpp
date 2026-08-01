#include "mainwindow.h"
#include "queue_models.h"
#include "utilities.h"

#include <QAbstractButton>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();

    connect(calculateButton, &QPushButton::clicked,
            this, &MainWindow::calculate);

    connect(clearButton, &QPushButton::clicked,
            this, &MainWindow::clearFields);

    connect(modelGroup, &QButtonGroup::buttonClicked,
            this, &MainWindow::changeModel);

    changeModel();
}

MainWindow::~MainWindow()
{
}

// ==========================================================================
// createModelTile
// --------------------------------------------------------------------------
// Builds one clickable, checkable tile button for the model picker grid.
// Registers it in modelGroup (an exclusive QButtonGroup) so only one
// tile can be selected at a time, like a set of radio buttons.
// ==========================================================================
QPushButton *MainWindow::createModelTile(const QString &name, const QString &shortLabel,
                                          const QString &modelValue)
{
    QPushButton *tile = new QPushButton();
    tile->setObjectName("modelTile");
    tile->setCheckable(true);
    tile->setCursor(Qt::PointingHandCursor);
    tile->setProperty("modelValue", modelValue);
    tile->setMinimumHeight(64);

    QVBoxLayout *tileLayout = new QVBoxLayout(tile);
    tileLayout->setContentsMargins(10, 10, 10, 10);
    tileLayout->setSpacing(2);
    tileLayout->setAlignment(Qt::AlignCenter);

    QLabel *nameLabel = new QLabel(name);
    nameLabel->setObjectName("tileName");
    nameLabel->setAlignment(Qt::AlignCenter);

    QLabel *subLabel = new QLabel(shortLabel);
    subLabel->setObjectName("tileSub");
    subLabel->setAlignment(Qt::AlignCenter);

    tileLayout->addWidget(nameLabel);
    tileLayout->addWidget(subLabel);

    modelGroup->addButton(tile);
    return tile;
}

// ==========================================================================
// createMetricCard
// --------------------------------------------------------------------------
// Builds one result card: a colored left-border accent (no icon glyph),
// a title, a large bold value, and a small grey caption. Returns the
// card frame (for show/hide control) and writes the value QLabel
// pointer into "valueLabelOut" so calculate()/showError() can update
// the displayed number later.
// ==========================================================================
QFrame *MainWindow::createMetricCard(const QString &accentColor, const QString &title, const QString &caption,
                                      const QString &initialValue, QLabel *&valueLabelOut,
                                      bool withProgress, QProgressBar *&progressBarOut)
{
    QFrame *card = new QFrame();
    card->setObjectName("metricCard");
    card->setStyleSheet(QString(
        "QFrame#metricCard { background: #FFFFFF; border: 1px solid #E5E7EF; "
        "border-left: 4px solid %1; border-radius: 12px; }"
    ).arg(accentColor));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(6);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("metricTitle");

    QLabel *valueLabel = new QLabel(initialValue);
    valueLabel->setObjectName("metricValue");

    QLabel *captionLabel = new QLabel(caption);
    captionLabel->setObjectName("metricCaption");

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);

    if (withProgress) {
        QProgressBar *bar = new QProgressBar();
        bar->setObjectName("metricProgress");
        bar->setRange(0, 100);
        bar->setValue(0);
        bar->setTextVisible(false);
        bar->setFixedHeight(6);
        cardLayout->addWidget(bar);
        progressBarOut = bar;
    } else {
        progressBarOut = nullptr;
    }

    cardLayout->addWidget(captionLabel);

    valueLabelOut = valueLabel;
    return card;
}

// ==========================================================================
// currentModel
// ==========================================================================
QString MainWindow::currentModel() const
{
    QAbstractButton *checked = modelGroup->checkedButton();
    if (checked) {
        return checked->property("modelValue").toString();
    }
    return "M/M/1";
}

// ==========================================================================
// modelDescriptionFor
// ==========================================================================
QString MainWindow::modelDescriptionFor(const QString &model) const
{
    if (model == "M/M/1")
        return "•  Poisson arrivals\n•  Exponential service times\n•  Single server\n•  Infinite capacity, FCFS discipline";
    if (model == "M/M/s")
        return "•  Poisson arrivals\n•  Exponential service times\n•  s parallel servers\n•  Infinite capacity, FCFS discipline";
    if (model == "M/M/∞")
        return "•  Poisson arrivals\n•  Exponential service times\n•  Infinite servers\n•  Every arrival served immediately";
    if (model == "M/M/1/K")
        return "•  Poisson arrivals\n•  Exponential service times\n•  Single server\n•  Finite capacity K — blocks when full";
    if (model == "M/M/s/K")
        return "•  Poisson arrivals\n•  Exponential service times\n•  s servers\n•  Finite capacity K — blocks when full";
    if (model == "M/G/1")
        return "•  Poisson arrivals\n•  General service time distribution\n•  Single server\n•  Solved via Pollaczek–Khinchine formula";
    return "";
}

// ==========================================================================
// stabilityTitleFor / stabilityDetailFor
// ==========================================================================
QString MainWindow::stabilityTitleFor(const QString &model) const
{
    if (model == "M/M/1" || model == "M/G/1")
        return "For System Stability: ρ < 1  (λ < μ)";
    if (model == "M/M/s")
        return "For System Stability: ρ < 1  (λ < s × μ)";
    if (model == "M/M/∞")
        return "This system is always stable";
    if (model == "M/M/1/K" || model == "M/M/s/K")
        return "This system is always stable";
    return "";
}

QString MainWindow::stabilityDetailFor(const QString &model) const
{
    if (model == "M/M/1" || model == "M/G/1")
        return "Arrival rate must be less than service rate for the system to be stable.";
    if (model == "M/M/s")
        return "Total arrival rate must be less than the combined capacity of all servers.";
    if (model == "M/M/∞")
        return "With infinite servers, every arrival is served immediately — no queue can ever form.";
    if (model == "M/M/1/K" || model == "M/M/s/K")
        return "The finite capacity K bounds the queue, so the system is stable regardless of λ vs μ.";
    return "";
}

void MainWindow::setupUI()
{
    resize(1080, 900);
    setMinimumSize(720, 560);
    setWindowTitle("Queueing Calculator");

    // ---- Scrollable container ----
    // All page content lives inside a QScrollArea so that on smaller
    // windows / lower-resolution screens the content scrolls instead of
    // being clipped or pushed off-screen. setWidgetResizable(true) lets
    // the inner content widget grow to fill the scroll area's width,
    // so cards/grids still stretch to use the available space.
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(scrollArea);

    QWidget *central = new QWidget();
    scrollArea->setWidget(central);

    QVBoxLayout *pageLayout = new QVBoxLayout(central);
    pageLayout->setContentsMargins(24, 20, 24, 20);
    pageLayout->setSpacing(16);

    // ================= Header bar =================
    QFrame *headerBar = new QFrame();
    headerBar->setObjectName("headerBar");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(14);

    QLabel *headerIcon = new QLabel("QC");
    headerIcon->setObjectName("headerIconText");
    headerIcon->setFixedSize(44, 44);
    headerIcon->setAlignment(Qt::AlignCenter);

    QVBoxLayout *headerText = new QVBoxLayout();
    headerText->setSpacing(2);
    QLabel *appTitle = new QLabel("Queueing Calculator");
    appTitle->setObjectName("appTitle");
    QLabel *appSubtitle = new QLabel("Analyze classical queueing models");
    appSubtitle->setObjectName("appSubtitle");
    headerText->addWidget(appTitle);
    headerText->addWidget(appSubtitle);

    QLabel *calcBadge = new QLabel("Analytical Calculator");
    calcBadge->setObjectName("pillBadge");

    headerLayout->addWidget(headerIcon);
    headerLayout->addLayout(headerText);
    headerLayout->addStretch();
    headerLayout->addWidget(calcBadge);

    pageLayout->addWidget(headerBar);

    // ================= Section 01: Model Parameters =================
    QHBoxLayout *section1Header = new QHBoxLayout();
    section1Header->setSpacing(12);
    QLabel *badge1 = new QLabel("01");
    badge1->setObjectName("sectionBadge");
    badge1->setFixedSize(38, 32);
    badge1->setAlignment(Qt::AlignCenter);

    QVBoxLayout *section1Text = new QVBoxLayout();
    section1Text->setSpacing(1);
    QLabel *section1Title = new QLabel("Model Parameters");
    section1Title->setObjectName("sectionTitle");
    QLabel *section1Sub = new QLabel("Configure your queueing model");
    section1Sub->setObjectName("sectionSubtitle");
    section1Text->addWidget(section1Title);
    section1Text->addWidget(section1Sub);

    QLabel *analyticalBadge1 = new QLabel("All values are analytical");
    analyticalBadge1->setObjectName("pillBadge");

    section1Header->addWidget(badge1);
    section1Header->addLayout(section1Text);
    section1Header->addStretch();
    section1Header->addWidget(analyticalBadge1);
    pageLayout->addLayout(section1Header);

    // ---- Two side-by-side cards: Queue Model | Arrival & Service ----
    QHBoxLayout *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(18);

    // ---- Left card: Queue Model (tile grid) ----
    QFrame *modelCard = new QFrame();
    modelCard->setObjectName("panelCard");
    QVBoxLayout *modelCardLayout = new QVBoxLayout(modelCard);
    modelCardLayout->setContentsMargins(20, 18, 20, 18);
    modelCardLayout->setSpacing(10);

    QLabel *modelCardTitle = new QLabel("Queue Model");
    modelCardTitle->setObjectName("cardTitle");
    QLabel *modelCardSub = new QLabel("Choose from classic queueing models");
    modelCardSub->setObjectName("cardSubtitle");
    modelCardLayout->addWidget(modelCardTitle);
    modelCardLayout->addWidget(modelCardSub);

    modelGroup = new QButtonGroup(this);
    modelGroup->setExclusive(true);

    QGridLayout *tileGrid = new QGridLayout();
    tileGrid->setSpacing(10);

    QPushButton *tile1 = createModelTile("M/M/1", "Single Server", "M/M/1");
    QPushButton *tile2 = createModelTile("M/M/s", "Multiple Servers", "M/M/s");
    QPushButton *tile3 = createModelTile("M/M/∞", "Infinite Servers", "M/M/∞");
    QPushButton *tile4 = createModelTile("M/M/1/K", "Finite Capacity", "M/M/1/K");
    QPushButton *tile5 = createModelTile("M/M/s/K", "Multi + Finite", "M/M/s/K");
    QPushButton *tile6 = createModelTile("M/G/1", "General Service", "M/G/1");

    tileGrid->addWidget(tile1, 0, 0);
    tileGrid->addWidget(tile2, 0, 1);
    tileGrid->addWidget(tile3, 0, 2);
    tileGrid->addWidget(tile4, 1, 0);
    tileGrid->addWidget(tile5, 1, 1);
    tileGrid->addWidget(tile6, 1, 2);

    tile1->setChecked(true); // default selection

    modelCardLayout->addLayout(tileGrid);

    QFrame *descBox = new QFrame();
    descBox->setObjectName("infoBoxLight");
    QHBoxLayout *descLayout = new QHBoxLayout(descBox);
    descLayout->setContentsMargins(14, 12, 14, 12);
    descLayout->setSpacing(12);
    modelDescriptionLabel = new QLabel("");
    modelDescriptionLabel->setObjectName("descText");
    modelDescriptionLabel->setWordWrap(true);
    descLayout->addWidget(modelDescriptionLabel, 1);

    modelCardLayout->addWidget(descBox);
    modelCardLayout->addStretch();

    // ---- Right card: Arrival Rate & Service Rate ----
    QFrame *ratesCard = new QFrame();
    ratesCard->setObjectName("panelCard");
    QVBoxLayout *ratesCardLayout = new QVBoxLayout(ratesCard);
    ratesCardLayout->setContentsMargins(20, 18, 20, 18);
    ratesCardLayout->setSpacing(14);

    // -- Arrival Rate row --
    QHBoxLayout *lambdaLabelRow = new QHBoxLayout();
    lambdaLabelRow->setSpacing(10);
    QLabel *lambdaLabel = new QLabel("Arrival Rate (λ)");
    lambdaLabel->setObjectName("fieldLabel");
    lambdaLabelRow->addWidget(lambdaLabel);
    lambdaLabelRow->addStretch();

    lambdaEdit = new QLineEdit();
    lambdaEdit->setPlaceholderText("e.g. 5");
    lambdaEdit->setMinimumHeight(38);

    lambdaModeBox = new QComboBox();
    lambdaModeBox->setObjectName("plainDropdown");
    lambdaModeBox->addItem("Rate");
    lambdaModeBox->addItem("Mean Time");

    lambdaUnitBox = new QComboBox();
    lambdaUnitBox->setObjectName("plainDropdown");
    lambdaUnitBox->addItem("per Hour",   static_cast<int>(TimeUnit::Hours));
    lambdaUnitBox->addItem("per Minute", static_cast<int>(TimeUnit::Minutes));
    lambdaUnitBox->addItem("per Second", static_cast<int>(TimeUnit::Seconds));

    QHBoxLayout *lambdaInputRow = new QHBoxLayout();
    lambdaInputRow->setSpacing(8);
    lambdaInputRow->addWidget(lambdaEdit, 2);
    lambdaInputRow->addWidget(lambdaModeBox, 1);
    lambdaInputRow->addWidget(lambdaUnitBox, 1);

    // -- Service Rate row --
    QHBoxLayout *muLabelRow = new QHBoxLayout();
    muLabelRow->setSpacing(10);
    QLabel *muLabel = new QLabel("Service Rate (μ)");
    muLabel->setObjectName("fieldLabel");
    muLabelRow->addWidget(muLabel);
    muLabelRow->addStretch();

    muEdit = new QLineEdit();
    muEdit->setPlaceholderText("e.g. 8");
    muEdit->setMinimumHeight(38);

    muModeBox = new QComboBox();
    muModeBox->setObjectName("plainDropdown");
    muModeBox->addItem("Rate");
    muModeBox->addItem("Mean Time");

    muUnitBox = new QComboBox();
    muUnitBox->setObjectName("plainDropdown");
    muUnitBox->addItem("per Hour",   static_cast<int>(TimeUnit::Hours));
    muUnitBox->addItem("per Minute", static_cast<int>(TimeUnit::Minutes));
    muUnitBox->addItem("per Second", static_cast<int>(TimeUnit::Seconds));

    QHBoxLayout *muInputRow = new QHBoxLayout();
    muInputRow->setSpacing(8);
    muInputRow->addWidget(muEdit, 2);
    muInputRow->addWidget(muModeBox, 1);
    muInputRow->addWidget(muUnitBox, 1);

    // -- Extra fields (Servers / Capacity / Variance), shown per model --
    serverLabel = new QLabel("Number of Servers (s)");
    serverLabel->setObjectName("fieldLabel");
    serverEdit = new QLineEdit();
    serverEdit->setPlaceholderText("e.g. 3");
    serverEdit->setMinimumHeight(38);

    capacityLabel = new QLabel("System Capacity (K)");
    capacityLabel->setObjectName("fieldLabel");
    capacityEdit = new QLineEdit();
    capacityEdit->setPlaceholderText("e.g. 10");
    capacityEdit->setMinimumHeight(38);

    varianceLabel = new QLabel("Service Time Variance (σ²)");
    varianceLabel->setObjectName("fieldLabel");
    varianceEdit = new QLineEdit();
    varianceEdit->setPlaceholderText("e.g. 0.02");
    varianceEdit->setMinimumHeight(38);

    ratesCardLayout->addLayout(lambdaLabelRow);
    ratesCardLayout->addLayout(lambdaInputRow);
    ratesCardLayout->addLayout(muLabelRow);
    ratesCardLayout->addLayout(muInputRow);
    ratesCardLayout->addWidget(serverLabel);
    ratesCardLayout->addWidget(serverEdit);
    ratesCardLayout->addWidget(capacityLabel);
    ratesCardLayout->addWidget(capacityEdit);
    ratesCardLayout->addWidget(varianceLabel);
    ratesCardLayout->addWidget(varianceEdit);
    ratesCardLayout->addStretch();

    cardsRow->addWidget(modelCard, 1);
    cardsRow->addWidget(ratesCard, 1);
    pageLayout->addLayout(cardsRow);

    // ---- Stability info banner (shield icon + bold title + detail) ----
    QFrame *stabilityBanner = new QFrame();
    stabilityBanner->setObjectName("infoBoxBlue");
    QHBoxLayout *stabilityOuter = new QHBoxLayout(stabilityBanner);
    stabilityOuter->setContentsMargins(16, 12, 16, 12);
    stabilityOuter->setSpacing(12);
    QVBoxLayout *stabilityTextLayout = new QVBoxLayout();
    stabilityTextLayout->setSpacing(2);
    stabilityTitleLabel = new QLabel("");
    stabilityTitleLabel->setObjectName("stabilityTitle");
    stabilityDetailLabel = new QLabel("");
    stabilityDetailLabel->setObjectName("stabilityDetail");
    stabilityDetailLabel->setWordWrap(true);
    stabilityTextLayout->addWidget(stabilityTitleLabel);
    stabilityTextLayout->addWidget(stabilityDetailLabel);
    stabilityOuter->addLayout(stabilityTextLayout, 1);
    pageLayout->addWidget(stabilityBanner);

    // ---- Buttons ----
    calculateButton = new QPushButton("Calculate                                        →");
    calculateButton->setObjectName("primaryButton");
    calculateButton->setCursor(Qt::PointingHandCursor);
    calculateButton->setMinimumHeight(48);

    clearButton = new QPushButton("Clear All                                        →");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(48);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(14);
    buttonLayout->addWidget(calculateButton, 1);
    buttonLayout->addWidget(clearButton, 1);
    pageLayout->addLayout(buttonLayout);

    // ---- Status / error banner (pill-shaped, red) ----
    statusLabel = new QLabel("");
    statusLabel->setObjectName("statusBanner");
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    pageLayout->addWidget(statusLabel);

    // ================= Section 02: Results =================
    QHBoxLayout *section2Header = new QHBoxLayout();
    section2Header->setSpacing(12);
    QLabel *badge2 = new QLabel("02");
    badge2->setObjectName("sectionBadge");
    badge2->setFixedSize(38, 32);
    badge2->setAlignment(Qt::AlignCenter);

    QVBoxLayout *section2Text = new QVBoxLayout();
    section2Text->setSpacing(1);
    QLabel *section2Title = new QLabel("Results");
    section2Title->setObjectName("sectionTitle");
    QLabel *section2Sub = new QLabel("Performance measures");
    section2Sub->setObjectName("sectionSubtitle");
    section2Text->addWidget(section2Title);
    section2Text->addWidget(section2Sub);

    QLabel *analyticalBadge2 = new QLabel("All values are analytical");
    analyticalBadge2->setObjectName("pillBadge");

    section2Header->addWidget(badge2);
    section2Header->addLayout(section2Text);
    section2Header->addStretch();
    section2Header->addWidget(analyticalBadge2);
    pageLayout->addLayout(section2Header);

    // ---- Results panel card (wraps mini-header + metric grid) ----
    QFrame *resultsPanel = new QFrame();
    resultsPanel->setObjectName("panelCard");
    QVBoxLayout *resultsPanelLayout = new QVBoxLayout(resultsPanel);
    resultsPanelLayout->setContentsMargins(20, 18, 20, 18);
    resultsPanelLayout->setSpacing(14);

    QHBoxLayout *perfHeader = new QHBoxLayout();
    perfHeader->setSpacing(12);
    QVBoxLayout *perfText = new QVBoxLayout();
    perfText->setSpacing(1);
    QLabel *perfTitle = new QLabel("Performance Measures");
    perfTitle->setObjectName("cardTitle");
    QLabel *perfSub = new QLabel("Calculated performance metrics for the selected model");
    perfSub->setObjectName("cardSubtitle");
    perfText->addWidget(perfTitle);
    perfText->addWidget(perfSub);
    perfHeader->addLayout(perfText);
    perfHeader->addStretch();

    resultsPanelLayout->addLayout(perfHeader);

    resultsGrid = new QGridLayout();
    resultsGrid->setSpacing(16);

    QProgressBar *unusedBar = nullptr;

    rhoCard        = createMetricCard("#5B4FE9", "Utilization (ρ)",         "(λ / μ)",       "0.0000", rhoValue, true, rhoProgress);
    p0Card         = createMetricCard("#2F6FED", "Prob. Empty (P₀)",        "(Probability)", "0.0000", p0Value, false, unusedBar);
    lqCard         = createMetricCard("#12A594", "Avg. in Queue (Lq)",      "(Customers)",   "0.0000", lqValue, false, unusedBar);
    lCard          = createMetricCard("#E0447B", "Avg. in System (L)",      "(Customers)",   "0.0000", lValue, false, unusedBar);
    wqCard         = createMetricCard("#E08E1D", "Avg. Wait (Wq)",          "(Minutes)",     "0.0000", wqValue, false, unusedBar);
    wCard          = createMetricCard("#1FA34D", "Avg. Time in System (W)", "(Minutes)",     "0.0000", wValue, false, unusedBar);
    pBlockCard     = createMetricCard("#5B4FE9", "Blocking Prob. (Pblock)", "(Probability)", "N/A",     pBlockValue, false, unusedBar);
    throughputCard = createMetricCard("#12A594", "Throughput (λeff)",       "(Per Minute)",  "0.0000", throughputValue, true, throughputProgress);

    resultsGrid->addWidget(rhoCard,        0, 0);
    resultsGrid->addWidget(p0Card,         0, 1);
    resultsGrid->addWidget(lqCard,         0, 2);
    resultsGrid->addWidget(lCard,          0, 3);
    resultsGrid->addWidget(wqCard,         1, 0);
    resultsGrid->addWidget(wCard,          1, 1);
    resultsGrid->addWidget(pBlockCard,     1, 2);
    resultsGrid->addWidget(throughputCard, 1, 3);

    for (int col = 0; col < 4; ++col) {
        resultsGrid->setColumnStretch(col, 1);
    }

    resultsPanelLayout->addLayout(resultsGrid);
    pageLayout->addWidget(resultsPanel);

    // ---- Footer tip bar ----
    QFrame *tipBar = new QFrame();
    tipBar->setObjectName("tipBar");
    QHBoxLayout *tipLayout = new QHBoxLayout(tipBar);
    tipLayout->setContentsMargins(16, 12, 16, 12);
    tipLayout->setSpacing(8);
    tipLayout->setAlignment(Qt::AlignCenter);
    QLabel *tipText = new QLabel("Tip: results update instantly — try switching models or time units to compare scenarios.");
    tipText->setObjectName("tipText");
    tipLayout->addStretch();
    tipLayout->addWidget(tipText);
    tipLayout->addStretch();
    pageLayout->addWidget(tipBar);

    // ================= Stylesheet (light theme) =================
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background: #F3F4F8;
            color: #1F2430;
            font-family: "Segoe UI";
            font-size: 10.5pt;
        }

        #headerBar, #panelCard, #metricCard {
            background: #FFFFFF;
            border: 1px solid #E5E7EF;
            border-radius: 14px;
        }

        #headerIconText {
            background: #5B4FE9;
            color: #FFFFFF;
            font-weight: 700;
            font-size: 13pt;
            border-radius: 14px;
        }

        #appTitle {
            font-size: 17pt;
            font-weight: 700;
            color: #14161B;
            background: transparent;
        }
        #appSubtitle {
            font-size: 9.5pt;
            color: #767C8C;
            background: transparent;
        }

        #pillBadge {
            background: #EDEAFE;
            color: #5B4FE9;
            border-radius: 14px;
            padding: 7px 14px;
            font-weight: 600;
            font-size: 9pt;
        }

        #sectionBadge {
            background: #EDEAFE;
            color: #5B4FE9;
            font-weight: 700;
            font-size: 10.5pt;
            border-radius: 8px;
        }
        #sectionTitle {
            font-size: 13pt;
            font-weight: 700;
            color: #14161B;
            background: transparent;
        }
        #sectionSubtitle {
            font-size: 9.5pt;
            color: #767C8C;
            background: transparent;
        }

        #cardTitle {
            font-size: 12pt;
            font-weight: 700;
            color: #14161B;
            background: transparent;
        }
        #cardSubtitle {
            font-size: 9pt;
            color: #8A90A0;
            background: transparent;
        }

        #fieldLabel {
            font-size: 10pt;
            font-weight: 600;
            color: #2B2F3A;
            background: transparent;
        }

        /* ---- Model tiles (radio-button style selector) ---- */
        #modelTile {
            background: #FFFFFF;
            border: 1.5px solid #E5E7EF;
            border-radius: 12px;
        }
        #modelTile:hover {
            border: 1.5px solid #B9B2F7;
            background: #FAFAFF;
        }
        #modelTile:checked {
            background: #F1EFFE;
            border: 1.5px solid #5B4FE9;
        }
        #tileName {
            font-weight: 700;
            font-size: 10pt;
            color: #14161B;
            background: transparent;
        }
        #tileSub {
            font-size: 8pt;
            color: #8A90A0;
            background: transparent;
        }

        QLineEdit {
            background: #FFFFFF;
            border: 1px solid #DCDFE8;
            border-radius: 9px;
            padding: 6px 10px;
            color: #14161B;
        }
        QLineEdit:focus {
            border: 1px solid #5B4FE9;
        }

        #plainDropdown {
            background: #FFFFFF;
            color: #2B2F3A;
            border: 1px solid #DCDFE8;
            border-radius: 9px;
            padding: 5px 8px;
        }

        QComboBox::drop-down { border: none; width: 20px; }
        QComboBox QAbstractItemView {
            background: #FFFFFF;
            border: 1px solid #DCDFE8;
            selection-background-color: #EDEAFE;
            selection-color: #5B4FE9;
            outline: none;
        }

        #infoBoxLight {
            background: #F7F6FC;
            border: 1px solid #E9E7F6;
            border-radius: 10px;
        }
        #descText {
            color: #5B5F6E;
            font-size: 9.5pt;
            background: transparent;
        }

        #infoBoxBlue {
            background: #EEF4FF;
            border: 1px solid #D8E6FF;
            border-radius: 10px;
        }
        #stabilityTitle {
            color: #1B4FBF;
            font-weight: 700;
            font-size: 10.5pt;
            background: transparent;
        }
        #stabilityDetail {
            color: #3A63B8;
            font-size: 9pt;
            background: transparent;
        }

        #primaryButton {
            background: #5B4FE9;
            color: #FFFFFF;
            border: none;
            border-radius: 24px;
            font-weight: 700;
            font-size: 11pt;
            text-align: left;
            padding-left: 20px;
        }
        #primaryButton:hover { background: #6E63EE; }
        #primaryButton:pressed { background: #4C41D1; }

        #secondaryButton {
            background: #FFFFFF;
            color: #5B4FE9;
            border: 1px solid #5B4FE9;
            border-radius: 24px;
            font-weight: 700;
            font-size: 11pt;
            text-align: left;
            padding-left: 20px;
        }
        #secondaryButton:hover { background: #F5F3FF; }
        #secondaryButton:pressed { background: #EDEAFE; }

        #statusBanner {
            background: #FDEAEA;
            border: 1px solid #F7C9CB;
            border-radius: 22px;
            color: #C0292E;
            font-weight: 600;
            padding: 12px 18px;
        }

        #metricTitle {
            color: #2B2F3A;
            font-size: 9.5pt;
            font-weight: 600;
            background: transparent;
        }
        #metricValue {
            color: #14161B;
            font-size: 18pt;
            font-weight: 700;
            background: transparent;
        }
        #metricCaption {
            color: #9098A8;
            font-size: 8.5pt;
            background: transparent;
        }
        #metricProgress {
            background: #EDEEF2;
            border: none;
            border-radius: 3px;
        }
        #metricProgress::chunk {
            background: #5B4FE9;
            border-radius: 3px;
        }

        #tipBar {
            background: #F7F7FA;
            border-top: 1px solid #E5E7EF;
        }
        #tipText {
            color: #767C8C;
            font-size: 9.5pt;
            background: transparent;
        }
    )");
}

void MainWindow::changeModel()
{
    serverLabel->hide();
    serverEdit->hide();

    capacityLabel->hide();
    capacityEdit->hide();

    varianceLabel->hide();
    varianceEdit->hide();

    // Pblock only applies to finite-capacity models (M/M/1/K, M/M/s/K),
    // so hide its card by default and show it only for those two cases.
    pBlockCard->hide();

    QString model = currentModel();

    if(model == "M/M/s")
    {
        serverLabel->show();
        serverEdit->show();
    }
    else if(model == "M/M/1/K")
    {
        capacityLabel->show();
        capacityEdit->show();

        pBlockCard->show();
    }
    else if(model == "M/M/s/K")
    {
        serverLabel->show();
        serverEdit->show();

        capacityLabel->show();
        capacityEdit->show();

        pBlockCard->show();
    }
    else if(model == "M/G/1")
    {
        varianceLabel->show();
        varianceEdit->show();
    }

    // Update the model description box and the stability banner
    modelDescriptionLabel->setText(modelDescriptionFor(model));
    stabilityTitleLabel->setText(stabilityTitleFor(model));
    stabilityDetailLabel->setText(stabilityDetailFor(model));

    // Switching models invalidates any previously shown error/results,
    // so clear the status message when the user changes their selection.
    statusLabel->hide();
    statusLabel->setText("");
}

// ==========================================================================
// unitFromComboBox
// ==========================================================================
TimeUnit MainWindow::unitFromComboBox(QComboBox *box) const
{
    return static_cast<TimeUnit>(box->currentData().toInt());
}

// ==========================================================================
// isMeanFromComboBox
// ==========================================================================
bool MainWindow::isMeanFromComboBox(QComboBox *box) const
{
    return box->currentText() == "Mean Time";
}

// ==========================================================================
// showError
// ==========================================================================
void MainWindow::showError(const QString &message)
{
    statusLabel->setText("Error: " + message);
    statusLabel->show();

    rhoValue->setText("-");
    p0Value->setText("-");
    lqValue->setText("-");
    lValue->setText("-");
    wqValue->setText("-");
    wValue->setText("-");
    pBlockValue->setText("-");
    throughputValue->setText("-");

    if (rhoProgress) rhoProgress->setValue(0);
    if (throughputProgress) throughputProgress->setValue(0);
}

// ==========================================================================
// calculate
// ==========================================================================
void MainWindow::calculate()
{
    statusLabel->hide();
    statusLabel->setText("");

    QString model = currentModel();

    // ---- Parse the raw numbers the user typed for lambda and mu ----
    double lambdaRaw = 0.0;
    double muRaw = 0.0;

    if (!tryParseDouble(lambdaEdit->text().toStdString(), lambdaRaw) || !isPositiveNumber(lambdaRaw)) {
        showError("Please enter a valid positive number for Arrival Rate (λ).");
        return;
    }

    if (!tryParseDouble(muEdit->text().toStdString(), muRaw) || !isPositiveNumber(muRaw)) {
        showError("Please enter a valid positive number for Service Rate (μ).");
        return;
    }

    // ---- Standardize lambda and mu to "events per minute" ----
    double lambda = convertToRatePerMinute(lambdaRaw, isMeanFromComboBox(lambdaModeBox), unitFromComboBox(lambdaUnitBox));
    double mu     = convertToRatePerMinute(muRaw,     isMeanFromComboBox(muModeBox),     unitFromComboBox(muUnitBox));

    // ---- Dispatch to the correct model, parsing any extra fields it needs ----
    QueueResults result;

    if (model == "M/M/1")
    {
        result = calculateMM1(lambda, mu);
    }
    else if (model == "M/M/s")
    {
        int servers = 0;
        if (!tryParsePositiveInt(serverEdit->text().toStdString(), servers)) {
            showError("Please enter a valid positive integer for Number of Servers (s).");
            return;
        }
        result = calculateMMS(lambda, mu, servers);
    }
    else if (model == "M/M/∞")
    {
        result = calculateMMInfinite(lambda, mu);
    }
    else if (model == "M/M/1/K")
    {
        int capacity = 0;
        if (!tryParsePositiveInt(capacityEdit->text().toStdString(), capacity)) {
            showError("Please enter a valid positive integer for System Capacity (K).");
            return;
        }
        result = calculateMM1K(lambda, mu, capacity);
    }
    else if (model == "M/M/s/K")
    {
        int servers = 0;
        int capacity = 0;
        if (!tryParsePositiveInt(serverEdit->text().toStdString(), servers)) {
            showError("Please enter a valid positive integer for Number of Servers (s).");
            return;
        }
        if (!tryParsePositiveInt(capacityEdit->text().toStdString(), capacity)) {
            showError("Please enter a valid positive integer for System Capacity (K).");
            return;
        }
        result = calculateMMSK(lambda, mu, servers, capacity);
    }
    else if (model == "M/G/1")
    {
        double variance = 0.0;
        if (!tryParseDouble(varianceEdit->text().toStdString(), variance) || variance < 0.0) {
            showError("Please enter a valid non-negative number for Service Time Variance (σ²).");
            return;
        }
        result = calculateMG1(lambda, mu, variance);
    }

    // ---- Handle invalid/unstable systems reported by the model function ----
    if (!result.isValid) {
        showError(QString::fromStdString(result.errorMessage));
        return;
    }

    // ---- Display the computed performance measures, 4 decimal places ----
    rhoValue->setText(QString::fromStdString(formatMeasure(result.rho)));
    p0Value->setText(QString::fromStdString(formatMeasure(result.P0)));
    lqValue->setText(QString::fromStdString(formatMeasure(result.Lq)));
    lValue->setText(QString::fromStdString(formatMeasure(result.L)));
    wqValue->setText(QString::fromStdString(formatMeasure(result.Wq)));
    wValue->setText(QString::fromStdString(formatMeasure(result.W)));
    pBlockValue->setText(QString::fromStdString(formatMeasure(result.Pblock)));
    throughputValue->setText(QString::fromStdString(formatMeasure(result.throughput)));

    // ---- Update progress bars (Utilization % and Throughput efficiency %) ----
    if (rhoProgress) {
        int rhoPercent = static_cast<int>(qBound(0.0, result.rho * 100.0, 100.0));
        rhoProgress->setValue(rhoPercent);
    }
    if (throughputProgress) {
        // Efficiency = how much of the arriving traffic actually gets
        // through vs. how much arrived in the first place (lambda).
        double efficiency = (lambda > 0.0) ? (result.throughput / lambda) : 0.0;
        int effPercent = static_cast<int>(qBound(0.0, efficiency * 100.0, 100.0));
        throughputProgress->setValue(effPercent);
    }
}

void MainWindow::clearFields()
{
    lambdaEdit->clear();
    muEdit->clear();
    serverEdit->clear();
    capacityEdit->clear();
    varianceEdit->clear();

    lambdaModeBox->setCurrentIndex(0);
    lambdaUnitBox->setCurrentIndex(0);
    muModeBox->setCurrentIndex(0);
    muUnitBox->setCurrentIndex(0);

    rhoValue->setText("0.0000");
    p0Value->setText("0.0000");
    lqValue->setText("0.0000");
    lValue->setText("0.0000");
    wqValue->setText("0.0000");
    wValue->setText("0.0000");
    pBlockValue->setText("N/A");
    throughputValue->setText("0.0000");

    if (rhoProgress) rhoProgress->setValue(0);
    if (throughputProgress) throughputProgress->setValue(0);

    statusLabel->hide();
    statusLabel->setText("");
}