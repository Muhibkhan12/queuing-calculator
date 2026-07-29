#include "mainwindow.h"
#include "queue_models.h"
#include "utilities.h"

#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
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
// createMetricCard
// --------------------------------------------------------------------------
// Builds one small bordered "card" widget used in the results grid: a
// muted, uppercase-style title on top and a large bold value underneath.
// Returns the QFrame (so changeModel() can show/hide the whole card) and
// writes the value QLabel pointer into "valueLabelOut" so calculate()/
// showError() can update the displayed number later.
// ==========================================================================
QFrame *MainWindow::createMetricCard(const QString &title, const QString &initialValue,
                                      QLabel *&valueLabelOut)
{
    QFrame *card = new QFrame();
    card->setObjectName("metricCard");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("metricTitle");

    QLabel *valueLabel = new QLabel(initialValue);
    valueLabel->setObjectName("metricValue");

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);

    valueLabelOut = valueLabel;
    return card;
}

void MainWindow::setupUI()
{
    resize(1000,820);
    setWindowTitle("Queueing Calculator");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(16);

    // ---------------- Header ----------------
    QLabel *title = new QLabel("Queueing Calculator");
    title->setObjectName("appTitle");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Analytical performance measures for classic queueing models ");
    subtitle->setObjectName("appSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    // ---------------- Input panel ----------------
    QGroupBox *inputBox = new QGroupBox("Model & Parameters");

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
    mainLayout->addWidget(inputBox);

    // ---------------- Buttons ----------------
    calculateButton = new QPushButton("Calculate");
    calculateButton->setObjectName("primaryButton");
    calculateButton->setCursor(Qt::PointingHandCursor);
    calculateButton->setMinimumHeight(42);

    clearButton = new QPushButton("Clear");
    clearButton->setObjectName("secondaryButton");
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setMinimumHeight(42);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    buttonLayout->addWidget(calculateButton, 2);
    buttonLayout->addWidget(clearButton, 1);

    mainLayout->addLayout(buttonLayout);

    // ---------------- Status banner ----------------
    statusLabel = new QLabel("");
    statusLabel->setObjectName("statusBanner");
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    // ---------------- Results panel ----------------
    QGroupBox *resultBox = new QGroupBox("Results");

    resultsGrid = new QGridLayout();
    resultsGrid->setSpacing(14);
    resultsGrid->setContentsMargins(4, 10, 4, 4);

    rhoCard        = createMetricCard("Utilization (ρ)", "0.0000", rhoValue);
    p0Card         = createMetricCard("Prob. Empty (P₀)", "0.0000", p0Value);
    lqCard         = createMetricCard("Avg. in Queue (Lq)", "0.0000", lqValue);
    lCard          = createMetricCard("Avg. in System (L)", "0.0000", lValue);
    wqCard         = createMetricCard("Avg. Wait (Wq, min)", "0.0000", wqValue);
    wCard          = createMetricCard("Avg. Time in System (W, min)", "0.0000", wValue);
    pBlockCard     = createMetricCard("Blocking Prob. (Pblock)", "N/A", pBlockValue);
    throughputCard = createMetricCard("Throughput (per min)", "0.0000", throughputValue);

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
    mainLayout->addWidget(resultBox);

    // ---------------- Stylesheet (dark theme, refreshed) ----------------
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background: #14161a;
            color: #E9EAEC;
            font-family: "Segoe UI";
            font-size: 11pt;
        }

        #appTitle {
            font-size: 26pt;
            font-weight: 700;
            color: #FFFFFF;
            padding-top: 4px;
        }

        #appSubtitle {
            font-size: 10pt;
            color: #8A9099;
            padding-bottom: 6px;
        }

        QGroupBox {
            background: #1B1E24;
            border: 1px solid #2A2D35;
            border-radius: 14px;
            margin-top: 14px;
            font-weight: 600;
            font-size: 11pt;
            color: #C9CDD3;
            padding: 18px 14px 14px 14px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 16px;
            top: -4px;
            padding: 0 8px;
            color: #E9EAEC;
            background: #14161a;
        }

        QLabel {
            color: #C9CDD3;
        }

        QLineEdit, QComboBox {
            background: #22252C;
            border: 1px solid #33363F;
            border-radius: 8px;
            padding: 8px 10px;
            color: #F2F3F5;
            selection-background-color: #5B8DEF;
        }

        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #5B8DEF;
        }

        QLineEdit::placeholder {
            color: #5F6470;
        }

        QComboBox::drop-down {
            border: none;
            width: 22px;
        }

        QComboBox QAbstractItemView {
            background: #22252C;
            border: 1px solid #33363F;
            selection-background-color: #5B8DEF;
            outline: none;
            color: #F2F3F5;
        }

        /* ---- Buttons ---- */
        #primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #5B8DEF, stop:1 #7C6CF6);
            color: #FFFFFF;
            border: none;
            border-radius: 10px;
            font-weight: 700;
            font-size: 11pt;
        }
        #primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                        stop:0 #6C9AFF, stop:1 #8C7DFF);
        }
        #primaryButton:pressed {
            background: #4A78D6;
        }

        #secondaryButton {
            background: transparent;
            color: #C9CDD3;
            border: 1px solid #33363F;
            border-radius: 10px;
            font-weight: 600;
            font-size: 11pt;
        }
        #secondaryButton:hover {
            border: 1px solid #5B8DEF;
            color: #FFFFFF;
        }
        #secondaryButton:pressed {
            background: #22252C;
        }

        /* ---- Status banner (errors / unstable system) ---- */
        #statusBanner {
            background: rgba(255, 107, 107, 0.12);
            border-left: 4px solid #FF6B6B;
            border-radius: 8px;
            color: #FF8383;
            font-weight: 600;
            padding: 10px 14px;
        }

        /* ---- Metric cards ---- */
        #metricCard {
            background: #22252C;
            border: 1px solid #2E323B;
            border-radius: 12px;
        }

        #metricTitle {
            color: #8A9099;
            font-size: 9pt;
            font-weight: 600;
        }

        #metricValue {
            color: #FFFFFF;
            font-size: 19pt;
            font-weight: 700;
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