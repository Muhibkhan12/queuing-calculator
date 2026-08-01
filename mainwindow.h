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
class QButtonGroup;
class QProgressBar;
class QAbstractButton;

/*
 * MainWindow
 * --------------------------------------------------------------------------
 * The single window of the Queueing Model Calculator GUI (light theme,
 * tile-based model picker + card dashboard layout).
 *
 * Responsibilities:
 *   - Let the user pick a queueing model from a grid of clickable tiles
 *     (modelGroup) instead of a dropdown
 *   - Show only the input fields relevant to the selected model, update
 *     the model description bullet list, and update the stability banner
 *     text (changeModel())
 *   - On "Calculate", read + validate the input text, convert λ/μ to a
 *     standardized "events per minute" rate, call the matching
 *     calculate*() function from queue_models.h/.cpp, and display the
 *     returned QueueResults in the results metric cards (calculate())
 *   - On "Clear All", reset all input and result fields (clearFields())
 *
 * All the actual queueing-theory math lives in queue_models.h/.cpp, and
 * input parsing/formatting/time-conversion helpers live in
 * utilities.h/.cpp. This class only handles reading from / writing to
 * the Qt widgets.
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

    // Builds one clickable "model tile" button (name + short label),
    // registers it in modelGroup with "modelValue" as its checkable
    // property, and returns the button so it can be placed in the tile
    // grid layout.
    QPushButton *createModelTile(const QString &name, const QString &shortLabel,
                                  const QString &modelValue);

    // Builds one metric card for the results grid: a colored left-border
    // accent (set via "accentColor"), a title, a large value, and a
    // small grey caption underneath. If "withProgress" is true, a thin
    // progress bar is added under the value and its pointer is written
    // into "progressBarOut" so calculate() can update it.
    QFrame *createMetricCard(const QString &accentColor, const QString &title, const QString &caption,
                              const QString &initialValue, QLabel *&valueLabelOut,
                              bool withProgress, QProgressBar *&progressBarOut);

    // Shows an error message in the status banner and blanks out all
    // result fields (used when input is invalid or the system is unstable)
    void showError(const QString &message);

    // Reads a "Hours"/"Minutes"/"Seconds" QComboBox selection (stored as
    // item data) and returns the matching TimeUnit enum value.
    TimeUnit unitFromComboBox(QComboBox *box) const;

    // Reads a "Rate"/"Mean Time" QComboBox selection and returns true
    // if "Mean Time" is selected.
    bool isMeanFromComboBox(QComboBox *box) const;

    // Returns the currently selected model string (from whichever tile
    // button in modelGroup is checked), e.g. "M/M/1".
    QString currentModel() const;

    // Returns the bullet-point assumptions list for the selected model,
    // shown in the light info box under the model tile grid.
    QString modelDescriptionFor(const QString &model) const;

    // Returns {title, detail} text for the stability info banner.
    QString stabilityTitleFor(const QString &model) const;
    QString stabilityDetailFor(const QString &model) const;

    // ---- Model tile selector ----
    QButtonGroup *modelGroup;
    QLabel *modelDescriptionLabel;

    // ---- Rate inputs ----
    QLineEdit *lambdaEdit;
    QLineEdit *muEdit;
    QLineEdit *serverEdit;
    QLineEdit *capacityEdit;
    QLineEdit *varianceEdit;

    // Time-conversion controls for lambda (arrival) and mu (service).
    QComboBox *lambdaModeBox;   // "Rate" or "Mean Time"
    QComboBox *lambdaUnitBox;   // "per Hour" / "per Minute" / "per Second"
    QComboBox *muModeBox;       // "Rate" or "Mean Time"
    QComboBox *muUnitBox;       // "per Hour" / "per Minute" / "per Second"

    // Labels for the conditionally-shown input rows
    QLabel *serverLabel;
    QLabel *capacityLabel;
    QLabel *varianceLabel;

    // Stability banner (shield icon + bold title + detail line)
    QLabel *stabilityTitleLabel;
    QLabel *stabilityDetailLabel;

    // ---- Buttons ----
    QPushButton *calculateButton;
    QPushButton *clearButton;

    // ---- Results grid ----
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

    // Progress bars for the two cards that show one (Utilization, Throughput)
    QProgressBar *rhoProgress;
    QProgressBar *throughputProgress;

    // Status/error banner shown below the Calculate/Clear buttons
    QLabel *statusLabel;
};

#endif