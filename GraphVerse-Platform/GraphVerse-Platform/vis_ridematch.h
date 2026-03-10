#ifndef VIS_RIDEMATCH_H
#define VIS_RIDEMATCH_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <vector>

class VisRideMatch : public QWidget {
  Q_OBJECT
public:
  explicit VisRideMatch(QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void onModePassenger();
  void onModeDriver();
  void onGenerateSample();
  void onRunMatching();
  void onClear();

private:
  enum class Mode { None, AddPassenger, AddDriver };
  struct Passenger {
    QPoint pos;
    int id;
  };
  struct Driver {
    QPoint pos;
    int id;
  };

  QWidget *m_toolbar;
  QLabel *m_statusLabel;
  QLabel *m_hintLabel;
  QPushButton *m_btnPassenger;
  QPushButton *m_btnDriver;
  QPushButton *m_btnSample;
  QPushButton *m_btnMatch;
  QPushButton *m_btnClear;

  Mode m_mode;
  std::vector<Passenger> m_passengers;
  std::vector<Driver> m_drivers;
  std::vector<std::pair<int, int>> m_candidateEdges;
  std::vector<std::pair<int, int>> m_matchedPairs;
  int m_nextPassengerId;
  int m_nextDriverId;
  int m_matchDistance;

  void setupUi();
  QRect canvasRect() const;
  void recomputeCandidates();
  int runFordFulkerson();
  void updateStatus();
};

#endif
