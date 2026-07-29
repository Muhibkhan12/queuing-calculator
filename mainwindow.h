#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QGroupBox;

/*
 * MainWindow
 * --------------------------------------------------------------------------
 * The single window of the Queueing Model Calculator GUI.
 *
 * Responsibilities:
 *   - Let the user pick a queueing model from a dropdown (modelBox)
 *   - Show only the input fields relevant to the selected model
 *     (changeModel() hides/shows fields like Servers, Capacity, Variance)
 *   - On "Calculate", read + validate the input text, call the matching
 *     calculate*() function from queue_models.h/.cpp, and display the
 *     returned QueueResults in the results panel (calculate())
 *   - On "Clear", reset all input and result fields (clearFields())
 *
 * All the actual queueing-theory math lives in queue_models.h/.cpp, and
 * input parsing/formatting helpers live in utilities.h/.cpp. This class
 * only handles reading from / writing to the Qt widgets.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void calculate();
    void clearFields();
    void changeModel();

private:
    void setupUI();

    // Shows an error message in the status label and blanks out all
    // result fields (used when input is invalid or the system is unstable)
    void showError(const QString &message);

    // Input
    QComboBox *modelBox;

    QLineEdit *lambdaEdit;
    QLineEdit *muEdit;
    QLineEdit *serverEdit;
    QLineEdit *capacityEdit;
    QLineEdit *varianceEdit;

    // Labels
    QLabel *serverLabel;
    QLabel *capacityLabel;
    QLabel *varianceLabel;

    // Buttons
    QPushButton *calculateButton;
    QPushButton *clearButton;

    // Results
    QLabel *rhoValue;
    QLabel *p0Value;
    QLabel *lqValue;
    QLabel *lValue;
    QLabel *wqValue;
    QLabel *wValue;
    QLabel *pBlockValue;
    QLabel *throughputValue;

    // Result row labels (kept as members so they can be hidden/shown
    // alongside their value labels, e.g. Pblock only matters for
    // finite-capacity models)
    QLabel *pBlockRowLabel;
    QLabel *throughputRowLabel;

    // Status/error message shown below the Calculate/Clear buttons
    QLabel *statusLabel;
};

#endif