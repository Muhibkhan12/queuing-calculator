#include "mainwindow.h"
#include "queue_models.h"
#include "utilities.h"

#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
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

    connect(modelBox, &QComboBox::currentTextChanged,
            this, &MainWindow::changeModel);

    changeModel();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    resize(950,780);
    setWindowTitle("Queueing Calculator");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QLabel *title = new QLabel("Queueing Calculator");

    QFont font;
    font.setPointSize(22);
    font.setBold(true);

    title->setFont(font);
    title->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(title);

    QGroupBox *inputBox = new QGroupBox("Input");

    QFormLayout *form = new QFormLayout();

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
    muEdit = new QLineEdit();
    serverEdit = new QLineEdit();
    capacityEdit = new QLineEdit();
    varianceEdit = new QLineEdit();

    // ---- Time-conversion controls ----
    // Each of lambda and mu can independently be entered as a "Rate"
    // (events per unit time) or a "Mean Time" (average time per event),
    // and in whichever time unit is convenient (Hours/Minutes/Seconds).
    // convertToRatePerMinute() in utilities.cpp standardizes both to a
    // common "events per minute" rate before any formula runs.
    lambdaModeBox = new QComboBox();
    lambdaModeBox->addItems({"Rate", "Mean Time"});

    lambdaUnitBox = new QComboBox();
    lambdaUnitBox->addItems({"Hours", "Minutes", "Seconds"});

    muModeBox = new QComboBox();
    muModeBox->addItems({"Rate", "Mean Time"});

    muUnitBox = new QComboBox();
    muUnitBox->addItems({"Hours", "Minutes", "Seconds"});

    serverLabel = new QLabel("Servers");
    capacityLabel = new QLabel("Capacity");
    varianceLabel = new QLabel("Variance");

    form->addRow("Queue Model", modelBox);

    // Arrival rate row, with its mode/unit dropdowns placed right next
    // to the input box so it's clear they modify how λ is interpreted.
    QHBoxLayout *lambdaRow = new QHBoxLayout();
    lambdaRow->addWidget(lambdaEdit);
    lambdaRow->addWidget(lambdaModeBox);
    lambdaRow->addWidget(lambdaUnitBox);
    form->addRow("Arrival (λ)", lambdaRow);

    // Service rate row, same idea.
    QHBoxLayout *muRow = new QHBoxLayout();
    muRow->addWidget(muEdit);
    muRow->addWidget(muModeBox);
    muRow->addWidget(muUnitBox);
    form->addRow("Service (μ)", muRow);

    form->addRow(serverLabel, serverEdit);
    form->addRow(capacityLabel, capacityEdit);
    form->addRow(varianceLabel, varianceEdit);

    inputBox->setLayout(form);

    mainLayout->addWidget(inputBox);

    calculateButton = new QPushButton("Calculate");
    clearButton = new QPushButton("Clear");

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(calculateButton);
    buttonLayout->addWidget(clearButton);

    mainLayout->addLayout(buttonLayout);

    // Status label shows validation / stability error messages.
    // Starts empty and hidden; showError() fills it in and makes it visible.
    statusLabel = new QLabel("");
    statusLabel->setStyleSheet("color:#FF6B6B; font-weight:bold;");
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    QGroupBox *resultBox = new QGroupBox("Results");

    QFormLayout *resultLayout = new QFormLayout();

    rhoValue = new QLabel("0.0000");
    p0Value  = new QLabel("0.0000");
    lqValue  = new QLabel("0.0000");
    lValue   = new QLabel("0.0000");
    wqValue  = new QLabel("0.0000");
    wValue   = new QLabel("0.0000");
    pBlockValue     = new QLabel("N/A");
    throughputValue = new QLabel("0.0000");

    resultLayout->addRow("ρ  (Utilization)", rhoValue);
    resultLayout->addRow("P₀  (Prob. Empty)", p0Value);
    resultLayout->addRow("Lq  (Avg. in Queue)", lqValue);
    resultLayout->addRow("L  (Avg. in System)", lValue);
    // Wq/W are computed in minutes internally (since lambda/mu are
    // standardized to "events per minute" before the formulas run),
    // so the row labels say so explicitly.
    resultLayout->addRow("Wq  (Avg. Wait, minutes)", wqValue);
    resultLayout->addRow("W  (Avg. Time in System, minutes)", wValue);

    // Keep references to the row's own QLabel (the "field label" Qt
    // creates for addRow) so changeModel() can hide/show the whole row.
    pBlockRowLabel = new QLabel("Pblock  (Blocking Prob.)");
    resultLayout->addRow(pBlockRowLabel, pBlockValue);

    throughputRowLabel = new QLabel("Throughput  (per minute)");
    resultLayout->addRow(throughputRowLabel, throughputValue);

    resultBox->setLayout(resultLayout);

    mainLayout->addWidget(resultBox);
        // ---------- Dark Theme ----------
    setStyleSheet(R"(
        QMainWindow{
            background:#202124;
        }

        QWidget{
            background:#202124;
            color:white;
            font-family:Segoe UI;
            font-size:12pt;
        }

        QGroupBox{
            border:1px solid #555;
            border-radius:10px;
            margin-top:10px;
            font-weight:bold;
            padding-top:10px;
        }

        QGroupBox::title{
            subcontrol-origin:margin;
            left:12px;
            padding:0 5px;
        }

        QLineEdit,QComboBox{
            background:#2d2d30;
            border:1px solid #555;
            border-radius:6px;
            padding:6px;
        }

        QPushButton{
            background:#0078D7;
            color:white;
            border:none;
            border-radius:6px;
            padding:8px;
            font-weight:bold;
        }

        QPushButton:hover{
            background:#2893FF;
        }

        QPushButton:pressed{
            background:#005A9E;
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
    // so hide it by default and show it only for those two cases below.
    pBlockRowLabel->hide();
    pBlockValue->hide();

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

        pBlockRowLabel->show();
        pBlockValue->show();
    }
    else if(model == "M/M/s/K")
    {
        serverLabel->show();
        serverEdit->show();

        capacityLabel->show();
        capacityEdit->show();

        pBlockRowLabel->show();
        pBlockValue->show();
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
// Displays "message" in the red status label and blanks out every result
// field so stale numbers from a previous (valid) calculation are never
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
        showError("Please enter a valid positive number for Arrival (λ).");
        return;
    }

    if (!tryParseDouble(muEdit->text().toStdString(), muRaw) || !isPositiveNumber(muRaw)) {
        showError("Please enter a valid positive number for Service (μ).");
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