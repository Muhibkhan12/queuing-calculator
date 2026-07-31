#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "utilities.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QGroupBox;
class QFrame;
class QGridLayout;
class QHBoxLayout;

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

// Builds one "metric card" (a small bordered panel showing a label
// and a big value) for the results grid. "accent" picks the pastel
// left-border/value-color theme (e.g. "purple", "blue", "green",
// "orange", "pink", "teal", "gray") matching the reference design's
// colored icon-chip pattern. Returns the card frame (for show/hide
// control) and writes the value QLabel* into "valueLabelOut" so
// calculate()/showError() can update it later.
QFrame *createMetricCard(const QString &title, const QString &initialValue,
                            QLabel *&valueLabelOut, const QString &accent);

// Builds the numbered "01  Section Title / subtitle" row used above
// each major panel (Model Parameters, Results) — a rounded number
// badge next to a bold title and a muted description.
QHBoxLayout *makeSectionHeader(const QString &number, const QString &sectionTitle,
                                const QString &sectionSubtitle);

// Shows an error message in the status banner and blanks out all
// result fields (used when input is invalid or the system is unstable)
void showError(const QString &message);

// Reads a "Hours"/"Minutes"/"Seconds" QComboBox selection and
// returns the matching TimeUnit enum value (used for lambdaUnitBox
// and muUnitBox before calling convertToRatePerMinute()).
TimeUnit unitFromComboBox(QComboBox *box) const;

// Reads a "Rate"/"Mean Time" QComboBox selection and returns true
// if "Mean Time" is selected (used for lambdaModeBox and muModeBox
// before calling convertToRatePerMinute()).
bool isMeanFromComboBox(QComboBox *box) const;

// Input
QComboBox *modelBox;

QLineEdit *lambdaEdit;
QLineEdit *muEdit;
QLineEdit *serverEdit;
QLineEdit *capacityEdit;
QLineEdit *varianceEdit;

// Time-conversion controls for lambda (arrival) and mu (service).
// Each has its own "mode" (Rate vs Mean Time) and "unit"
// (Hours/Minutes/Seconds) so lambda and mu can be entered in
// completely different units and still be converted to a
// consistent internal rate before the formulas run.
QComboBox *lambdaModeBox;   // "Rate" or "Mean Time"
QComboBox *lambdaUnitBox;   // "Hours" / "Minutes" / "Seconds"
QComboBox *muModeBox;       // "Rate" or "Mean Time"
QComboBox *muUnitBox;       // "Hours" / "Minutes" / "Seconds"

// Labels for the conditionally-shown input rows
QLabel *serverLabel;
QLabel *capacityLabel;
QLabel *varianceLabel;

// Buttons
QPushButton *calculateButton;
QPushButton *clearButton;

// Results grid layout, so changeModel() can add/remove metric
// cards without needing to know exact row/column positions.
QGridLayout *resultsGrid;

// Result metric cards (the bordered box widgets)
QFrame *rhoCard;
QFrame *p0Card;
QFrame *lqCard;
QFrame *lCard;
QFrame *wqCard;
QFrame *wCard;
QFrame *pBlockCard;
QFrame *throughputCard;

// Result values (the big number QLabel inside each card)
QLabel *rhoValue;
QLabel *p0Value;
QLabel *lqValue;
QLabel *lValue;
QLabel *wqValue;
QLabel *wValue;
QLabel *pBlockValue;
QLabel *throughputValue;

// Status/error banner shown below the Calculate/Clear buttons
QLabel *statusLabel;
};

#endif