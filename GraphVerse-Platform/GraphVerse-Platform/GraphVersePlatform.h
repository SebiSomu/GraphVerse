#pragma once

#include "vis_components.h"
#include "vis_flow.h"
#include "vis_friends.h"
#include "vis_maze.h"
#include "vis_mst.h"
#include "vis_ridematch.h"
#include "vis_shortest.h"
#include "vis_supermarket.h"
#include "vis_translation.h"
#include <QtCore/QEasingCurve>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QEnterEvent>
#include <QtGui/QFont>
#include <QtGui/QLinearGradient>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class AnimatedButton : public QPushButton {
  Q_OBJECT
  Q_PROPERTY(float glowOpacity READ glowOpacity WRITE setGlowOpacity)

public:
  explicit AnimatedButton(const QString &text, QWidget *parent = nullptr);
  float glowOpacity() const { return m_glowOpacity; }
  void setGlowOpacity(float v);
  void setGlowColor(const QColor &c) {
    m_glowColor = c;
    update();
  }

protected:
  void enterEvent(QEnterEvent *e) override;
  void leaveEvent(QEvent *e) override;
  void paintEvent(QPaintEvent *e) override;

private:
  float m_glowOpacity = 0.0f;
  QColor m_glowColor;
};

// Big clickable card for navigation
class BigCategoryButton : public QFrame {
  Q_OBJECT

public:
  explicit BigCategoryButton(const QString &title, const QString &subtitle,
                             const QString &iconText, const QColor &accentColor,
                             QWidget *parent = nullptr);

signals:
  void clicked();

protected:
  void paintEvent(QPaintEvent *e) override;
  void enterEvent(QEnterEvent *e) override;
  void leaveEvent(QEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

private:
  QString m_title;
  QString m_subtitle;
  QString m_iconText;
  QColor m_accentColor;
  bool m_hovered = false;
};

class GraphCanvas : public QWidget {
  Q_OBJECT
public:
  explicit GraphCanvas(QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *e) override;
  void timerEvent(QTimerEvent *e) override;

private:
  struct Node {
    float x, y, vx, vy;
  };
  QVector<Node> m_nodes;
  QVector<QPair<int, int>> m_edges;
  float m_time = 0.0f;
  int m_timer = 0;
};

// Hero widget that keeps canvas stretched
class HeroWidget : public QWidget {
  Q_OBJECT
public:
  explicit HeroWidget(QWidget *parent = nullptr) : QWidget(parent) {}
  void setCanvas(GraphCanvas *c) { m_canvas = c; }

protected:
  void resizeEvent(QResizeEvent *e) override {
    QWidget::resizeEvent(e);
    if (m_canvas)
      m_canvas->resize(size());
  }

private:
  GraphCanvas *m_canvas = nullptr;
};

class GraphVersePlatform : public QMainWindow {
  Q_OBJECT

public:
  explicit GraphVersePlatform(QWidget *parent = nullptr);
  ~GraphVersePlatform();

private:
  void setupUi();
  void applyGlobalStyle();

  // Navigation
  void navigateToPlaceholder(const QString &title, int returnIndex);

  // UI Builders
  QWidget *buildHeader();
  QWidget *buildFooter();
  HeroWidget *buildHeroSection();

  // Page Builders
  QWidget *buildHomeView();
  QWidget *buildVisualisersView();
  QWidget *buildAppsView();
  QWidget *buildPlaceholderView();

  // Components
  QStackedWidget *m_stack = nullptr;
  GraphCanvas *m_canvas = nullptr;

  // Placeholder Page Components
  QWidget *m_placeholderPage = nullptr;
  QLabel *m_placeholderTitleLabel = nullptr;
  AnimatedButton *m_placeholderBackBtn = nullptr;

  // Visualiser Widgets
  VisMaze *m_visMaze = nullptr;
  VisComponents *m_visComponents = nullptr;
  VisMST *m_visMST = nullptr;
  VisShortest *m_visShortest = nullptr;
  VisFlow *m_visFlow = nullptr;

  // Real World Apps Widgets
  VisFriends *m_visFriends = nullptr;
  VisTranslation *m_visTranslation = nullptr;
  VisSupermarket *m_visSupermarket = nullptr;
  VisRideMatch *m_visRideMatch = nullptr;
};
