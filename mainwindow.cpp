#include "mainwindow.h"
#include "queue_models.h"
#include "utilities.h"

#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QFont>
#include <QSpacerItem>
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();

    connect(calculateButton, &QPushButton::clicked,
            this, &MainWindow::calculate);

    connect(clearButton, &QPushButton::clicked,
            this, &MainWindow::clearFields);

    connect(modelBox, &QComboBox::currentTextChanged,
            this, &MainWindow::changeModel);

    changeModel();
}

MainWindow::~MainWindow()
{
}

// ==========================================================================
// addSoftShadow
// --------------------------------------------------------------------------
// Small local helper that applies the soft, low-opacity elevation shadow
// used throughout the reference design (cards, group boxes, buttons).
// Not declared in the header since it's only used inside this file.
// ==========================================================================
static void addSoftShadow(QWidget *w, int blur = 22, int yOffset = 6, int alpha = 40)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(w);
    shadow->setBlurRadius(blur);
    shadow->setOffset(0, yOffset);
    shadow->setColor(QColor(76, 81, 191, alpha));
    w->setGraphicsEffect(shadow);
}

// ==========================================================================
// createMetricCard
// --------------------------------------------------------------------------
// Builds one small bordered "card" widget used in the results grid: a
// muted, uppercase-style title on top and a large bold value underneath.
// "accent" sets a pastel-colored left border + tinted value color (mirrors
// the colored icon-chip pattern in the reference design — purple/blue/
// green/orange/pink/teal). Returns the QFrame (so changeModel() can show/
// hide the whole card) and writes the value QLabel pointer into
// "valueLabelOut" so calculate()/showError() can update the number later.
// ==========================================================================
QFrame *MainWindow::createMetricCard(const QString &title, const QString &initialValue,
                                      QLabel *&valueLabelOut, const QString &accent)
{
    QFrame *card = new QFrame();
    card->setObjectName("metricCard");
    card->setProperty("accent", accent);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(6);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("metricTitle");

    QLabel *valueLabel = new QLabel(initialValue);
    valueLabel->setObjectName("metricValue");
    valueLabel->setProperty("accent", accent);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);

    addSoftShadow(card, 18, 4, 22);

    valueLabelOut = valueLabel;
    return card;
}

void MainWindow::setupUI()
{
    resize(1040, 860);
    setWindowTitle("Queueing Calculator");

    QWidget *central = new QWidget(this);
    central->setObjectName("appBackground");
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    // ---------------- Header ----------------
    QFrame *headerCard = new QFrame();
    headerCard->setObjectName("headerCard");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(20, 16, 20, 16);
    headerLayout->setSpacing(4);

    QLabel *badge = new QLabel("Ξ");
    badge->setObjectName("headerBadge");
    badge->setFixedSize(48, 48);
    badge->setAlignment(Qt::AlignCenter);

    QVBoxLayout *titleBlock = new QVBoxLayout();
    titleBlock->setSpacing(2);

    QLabel *title = new QLabel("Queueing Calculator");
    title->setObjectName("appTitle");

    QLabel *subtitle = new QLabel("Analytical performance measures for classic queueing models");
    subtitle->setObjectName("appSubtitle");

    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);

    QLabel *tag = new QLabel("Analytical Calculator");
    tag->setObjectName("headerTag");
    tag->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(badge);
    headerLayout->addSpacing(12);
    headerLayout->addLayout(titleBlock, 1);
    headerLayout->addWidget(tag);

    addSoftShadow(headerCard, 24, 8, 18);
    mainLayout->addWidget(headerCard);

    // ---------------- Section 01 label ----------------
    mainLayout->addLayout(makeSectionHeader("01", "Model Parameters", "Configure your queueing model"));

    // ---------------- Input panel ----------------
    QGroupBox *inputBox = new QGroupBox();
    inputBox->setObjectName("panelCard");

    QFormLayout *form = new QFormLayout();
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(14);
    form->setLabelAlignment(Qt::AlignLeft);

    modelBox = new QComboBox();
    modelBox->addItems({
        "M/M/1",
        "M/M/s",
        "M/M/∞",
        "M/M/1/K",
        "M/M/s/K",
        "M/G/1"
    });

    lambdaEdit = new QLineEdit();
    lambdaEdit->setPlaceholderText("e.g. 5");
    muEdit = new QLineEdit();
    muEdit->setPlaceholderText("e.g. 8");
    serverEdit = new QLineEdit();
    serverEdit->setPlaceholderText("e.g. 3");
    capacityEdit = new QLineEdit();
    capacityEdit->setPlaceholderText("e.g. 10");
    varianceEdit = new QLineEdit();
    varianceEdit->setPlaceholderText("e.g. 0.02");

    // ---- Time-conversion controls ----
    // Each of lambda and mu can independently be entered as a "Rate"
    // (events per unit time) or a "Mean Time" (average time per event),
    // and in whichever time unit is convenient (Hours/Minutes/Seconds).
    // convertToRatePerMinute() in utilities.cpp standardizes both to a
    // common "events per minute" rate before any formula runs.
    lambdaModeBox = new QComboBox();
    lambdaModeBox->addItems({"Rate", "Mean Time"});
    lambdaModeBox->setFixedWidth(115);

    lambdaUnitBox = new QComboBox();
    lambdaUnitBox->addItems({"Hours", "Minutes", "Seconds"});
    lambdaUnitBox->setFixedWidth(100);

    muModeBox = new QComboBox();
    muModeBox->addItems({"Rate", "Mean Time"});
    muModeBox->setFixedWidth(115);

    muUnitBox = new QComboBox();
    muUnitBox->addItems({"Hours", "Minutes", "Seconds"});
    muUnitBox->setFixedWidth(100);

    serverLabel = new QLabel("Servers (s)");
    capacityLabel = new QLabel("Capacity (K)");
    varianceLabel = new QLabel("Variance (σ²)");

    form->addRow("Queue Model", modelBox);

    // Arrival rate row, with its mode/unit dropdowns placed right next
    // to the input box so it's clear they modify how λ is interpreted.
    QHBoxLayout *lambdaRow = new QHBoxLayout();
    lambdaRow->setSpacing(8);
    lambdaRow->addWidget(lambdaEdit, 1);
    lambdaRow->addWidget(lambdaModeBox);
    lambdaRow->addWidget(lambdaUnitBox);
    form->addRow("Arrival Rate (λ)", lambdaRow);

    // Service rate row, same idea.
    QHBoxLayout *muRow = new QHBoxLayout();
    muRow->setSpacing(8);
    muRow->addWidget(muEdit, 1);
    muRow->addWidget(muModeBox);
    muRow->addWidget(muUnitBox);
    form->addRow("Service Rate (μ)", muRow);

    form->addRow(serverLabel, serverEdit);
    form->addRow(capacityLabel, capacityEdit);
    form->addRow(varianceLabel, varianceEdit);

    inputBox->setLayout(form);
    addSoftShadow(inputBox, 20, 6, 16);
    mainLayout->addWidget(inputBox);

    // ---------------- Info callout (stability rule) ----------------
    QFrame *stabilityInfo = new QFrame();
    stabilityInfo->setObjectName("infoBanner");
    QVBoxLayout *stabilityLayout = new QVBoxLayout(stabilityInfo);
    stabilityLayout->setContentsMargins(16, 12, 16, 12);
    stabilityLayout->setSpacing(2);

    QLabel *stabilityTitle = new QLabel("For System Stability: ρ &lt; 1 (λ &lt; μ)");
    stabilityTitle->setObjectName("infoTitle");
    QLabel *stabilityBody = new QLabel("Arrival rate must be less than service rate for the system to be stable.");
    stabilityBody->setObjectName("infoBody");
    stabilityBody->setWordWrap(true);

    stabilityLayout->addWidget(stabilityTitle);
    stabilityLayout->addWidget(stabilityBody);
    mainLayout->addWidget(stabilityInfo);

    // ---------------- Buttons ----------------
    calculateButton = new QPushButton("⟶  Calculate");
    calculateButton->setObjectName("primaryButton");
    calculateButton->setCursor(Qt::PointingHandCursor);
    calculateButton->setMinimumHeight(46);

    clearButton = new QPushButton("⟲  Clear All");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(46);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    buttonLayout->addWidget(calculateButton, 2);
    buttonLayout->addWidget(clearButton, 1);

    addSoftShadow(calculateButton, 20, 8, 60);
    mainLayout->addLayout(buttonLayout);

    // ---------------- Status banner ----------------
    statusLabel = new QLabel("");
    statusLabel->setObjectName("statusBanner");
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    // ---------------- Section 02 label ----------------
    mainLayout->addLayout(makeSectionHeader("02", "Results", "Performance measures"));

    // ---------------- Results panel ----------------
    QGroupBox *resultBox = new QGroupBox();
    resultBox->setObjectName("panelCard");

    resultsGrid = new QGridLayout();
    resultsGrid->setSpacing(14);
    resultsGrid->setContentsMargins(4, 6, 4, 4);

    rhoCard        = createMetricCard("Utilization (ρ)", "0.0000", rhoValue, "purple");
    p0Card         = createMetricCard("Prob. Empty (P₀)", "0.0000", p0Value, "blue");
    lqCard         = createMetricCard("Avg. in Queue (Lq)", "0.0000", lqValue, "green");
    lCard          = createMetricCard("Avg. in System (L)", "0.0000", lValue, "orange");
    wqCard         = createMetricCard("Avg. Wait (Wq, min)", "0.0000", wqValue, "pink");
    wCard          = createMetricCard("Avg. Time in System (W, min)", "0.0000", wValue, "purple");
    pBlockCard     = createMetricCard("Blocking Prob. (Pblock)", "N/A", pBlockValue, "gray");
    throughputCard = createMetricCard("Throughput (per min)", "0.0000", throughputValue, "teal");

    // 4 cards per row, 2 rows
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

    resultBox->setLayout(resultsGrid);
    addSoftShadow(resultBox, 20, 6, 16);
    mainLayout->addWidget(resultBox);

    // ---------------- Stylesheet (light indigo SaaS-dashboard theme) ----------------
    setStyleSheet(R"(
        #appBackground {
            background: #F7F7FC;
        }

        QMainWindow, QWidget {
            color: #1F2430;
            font-family: "Segoe UI";
            font-size: 11pt;
        }

        /* ---- Header ---- */
        #headerCard {
            background: #FFFFFF;
            border: 1px solid #ECECF5;
            border-radius: 18px;
        }

        #headerBadge {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                        stop:0 #6366F1, stop:1 #7C6CF6);
            border-radius: 14px;
            color: #FFFFFF;
            font-size: 20pt;
            font-weight: 700;
        }

        #appTitle {
            font-size: 20pt;
            font-weight: 800;
            color: #16181F;
        }

        #appSubtitle {
            font-size: 10pt;
            color: #8B8FA3;
        }

        #headerTag {
            background: #EEEEFB;
            color: #5B54E8;
            border-radius: 16px;
            padding: 8px 16px;
            font-weight: 600;
            font-size: 9.5pt;
        }

        /* ---- Section numbering ---- */
        #sectionNumber {
            background: #ECEAFE;
            color: #5B54E8;
            border-radius: 12px;
            font-size: 14pt;
            font-weight: 800;
        }

        #sectionTitle {
            font-size: 14pt;
            font-weight: 800;
            color: #16181F;
        }

        #sectionSubtitle {
            font-size: 9.5pt;
            color: #8B8FA3;
        }

        /* ---- Panels ---- */
        QGroupBox#panelCard {
            background: #FFFFFF;
            border: 1px solid #ECECF5;
            border-radius: 18px;
            padding: 18px 14px 14px 14px;
        }

        QLabel {
            color: #4B4F5C;
        }

        QLineEdit, QComboBox {
            background: #FCFCFF;
            border: 1px solid #E3E4EF;
            border-radius: 10px;
            padding: 9px 10px;
            color: #1F2430;
            selection-background-color: #6366F1;
        }

        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #6366F1;
        }

        QLineEdit::placeholder {
            color: #B0B3C2;
        }

        QComboBox::drop-down {
            border: none;
            width: 22px;
        }

        QComboBox QAbstractItemView {
            background: #FFFFFF;
            border: 1px solid #E3E4EF;
            selection-background-color: #6366F1;
            selection-color: #FFFFFF;
            outline: none;
            color: #1F2430;
        }

        /* ---- Info banner (stability rule) ---- */
        #infoBanner {
            background: #EEF3FF;
            border-radius: 14px;
        }
        #infoTitle {
            color: #2E5CE6;
            font-weight: 700;
            font-size: 10.5pt;
        }
        #infoBody {
            color: #4C6FD6;
            font-size: 9.5pt;
        }

        /* ---- Buttons (pill-shaped) ---- */
        #primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #6366F1, stop:1 #7C6CF6);
            color: #FFFFFF;
            border: none;
            border-radius: 23px;
            font-weight: 700;
            font-size: 11pt;
        }
        #primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #7476F5, stop:1 #8C7DFF);
        }
        #primaryButton:pressed {
            background: #5350D6;
        }

        #secondaryButton {
            background: #FFFFFF;
            color: #4B4F5C;
            border: 1px solid #E3E4EF;
            border-radius: 23px;
            font-weight: 600;
            font-size: 11pt;
        }
        #secondaryButton:hover {
            border: 1px solid #6366F1;
            color: #6366F1;
        }
        #secondaryButton:pressed {
            background: #F7F7FC;
        }

        /* ---- Status banner (errors / unstable system) ---- */
        #statusBanner {
            background: #FDECEC;
            border-radius: 12px;
            color: #E4574C;
            font-weight: 600;
            padding: 12px 16px;
        }

        /* ---- Metric cards ---- */
        #metricCard {
            background: #FFFFFF;
            border: 1px solid #EDEDF5;
            border-radius: 16px;
        }
        #metricCard[accent="purple"] { border-left: 4px solid #8B7CF6; }
        #metricCard[accent="blue"]   { border-left: 4px solid #4C8DF6; }
        #metricCard[accent="green"]  { border-left: 4px solid #34C787; }
        #metricCard[accent="orange"] { border-left: 4px solid #F5A84C; }
        #metricCard[accent="pink"]   { border-left: 4px solid #F0679B; }
        #metricCard[accent="teal"]   { border-left: 4px solid #2CC1BE; }
        #metricCard[accent="gray"]   { border-left: 4px solid #A6A9B8; }

        #metricTitle {
            color: #8B8FA3;
            font-size: 9pt;
            font-weight: 600;
        }

        #metricValue {
            color: #16181F;
            font-size: 19pt;
            font-weight: 800;
        }
        #metricValue[accent="purple"] { color: #6C5CE0; }
        #metricValue[accent="blue"]   { color: #2F6FE0; }
        #metricValue[accent="green"]  { color: #1FA76B; }
        #metricValue[accent="orange"] { color: #E08F2C; }
        #metricValue[accent="pink"]   { color: #E0447F; }
        #metricValue[accent="teal"]   { color: #189B98; }
        #metricValue[accent="gray"]   { color: #6B6F80; }
    )");
}

// ==========================================================================
// makeSectionHeader
// --------------------------------------------------------------------------
// Builds the "01  Section Title / subtitle" row used above each major
// panel, mirroring the numbered-section pattern from the reference design
// (a rounded badge with the step number, next to a bold title and a
// muted description). Returns a QHBoxLayout ready to be added to the
// main vertical layout.
// ==========================================================================
QHBoxLayout *MainWindow::makeSectionHeader(const QString &number, const QString &sectionTitle,
                                            const QString &sectionSubtitle)
{
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(12);

    QLabel *numberBadge = new QLabel(number);
    numberBadge->setObjectName("sectionNumber");
    numberBadge->setFixedSize(40, 40);
    numberBadge->setAlignment(Qt::AlignCenter);

    QVBoxLayout *textBlock = new QVBoxLayout();
    textBlock->setSpacing(1);

    QLabel *titleLbl = new QLabel(sectionTitle);
    titleLbl->setObjectName("sectionTitle");

    QLabel *subLbl = new QLabel(sectionSubtitle);
    subLbl->setObjectName("sectionSubtitle");

    textBlock->addWidget(titleLbl);
    textBlock->addWidget(subLbl);

    row->addWidget(numberBadge);
    row->addLayout(textBlock, 1);

    return row;
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

    QString model = modelBox->currentText();

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

    // Switching models invalidates any previously shown error/results,
    // so clear the status message when the user changes their selection.
    statusLabel->hide();
    statusLabel->setText("");
}

// ==========================================================================
// unitFromComboBox
// --------------------------------------------------------------------------
// Maps the text of a "Hours"/"Minutes"/"Seconds" QComboBox to the
// matching TimeUnit enum value used by convertToRatePerMinute().
// ==========================================================================
TimeUnit MainWindow::unitFromComboBox(QComboBox *box) const
{
    QString text = box->currentText();
    if (text == "Hours") {
        return TimeUnit::Hours;
    } else if (text == "Seconds") {
        return TimeUnit::Seconds;
    }
    return TimeUnit::Minutes;
}

// ==========================================================================
// isMeanFromComboBox
// --------------------------------------------------------------------------
// Maps the text of a "Rate"/"Mean Time" QComboBox to a bool: true when
// the user selected "Mean Time" (so the entered number is an average
// time per event, not a rate).
// ==========================================================================
bool MainWindow::isMeanFromComboBox(QComboBox *box) const
{
    return box->currentText() == "Mean Time";
}

// ==========================================================================
// showError
// --------------------------------------------------------------------------
// Displays "message" in the red status banner and blanks out every result
// card so stale numbers from a previous (valid) calculation are never
// left on screen next to an error.
// ==========================================================================
void MainWindow::showError(const QString &message)
{
    statusLabel->setText(message);
    statusLabel->show();

    rhoValue->setText("-");
    p0Value->setText("-");
    lqValue->setText("-");
    lValue->setText("-");
    wqValue->setText("-");
    wValue->setText("-");
    pBlockValue->setText("-");
    throughputValue->setText("-");
}

// ==========================================================================
// calculate
// --------------------------------------------------------------------------
// Reads the text from the input fields, validates/parses it using the
// helpers in utilities.h, converts lambda and mu to a standardized
// "events per minute" rate (regardless of the Rate/Mean Time mode or
// Hours/Minutes/Seconds unit the user picked for each), calls the
// matching calculate*() function from queue_models.h for the currently
// selected model, and displays the returned QueueResults (or an error
// message if inputs/stability fail).
// ==========================================================================
void MainWindow::calculate()
{
    statusLabel->hide();
    statusLabel->setText("");

    QString model = modelBox->currentText();

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
    // This is where the Rate-vs-Mean-Time and Hours/Minutes/Seconds
    // conversion happens, so lambda and mu end up in the SAME unit
    // even if the user entered them completely differently (e.g.
    // lambda as "customers per hour" and mu as "mean minutes to serve").
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

    statusLabel->hide();
    statusLabel->setText("");
}