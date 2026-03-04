#include "GraphVersePlatform.h"
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QSpacerItem>
#include <QtGui/QPainterPath>
#include <QtGui/QFontDatabase>
#include <QtCore/QTimer>
#include <QtCore/QTimerEvent>
#include <QtWidgets/QApplication>
#include <QtGui/QMouseEvent>
#include <cmath>
#include <cstdlib>
#include <QPropertyAnimation>

// ─────────────────────────────────────────────────────────────
//  Colour palette
// ─────────────────────────────────────────────────────────────
namespace Palette {
    const QColor BG_DEEP    { 8,  10, 20  };
    const QColor BG_SURFACE { 14, 17, 32  };
    const QColor BG_CARD    { 18, 22, 42  };
    const QColor BG_CARD_H  { 22, 28, 56  };
    const QColor BORDER     { 45, 55, 100 };
    const QColor ACCENT_VIS { 99, 102, 241};   // indigo
    const QColor ACCENT_APP { 20, 184, 166};   // teal
    const QColor TEXT_PRI   { 235,237,255 };
    const QColor TEXT_SEC   { 140,148,190 };
}

// ─────────────────────────────────────────────────────────────
//  GraphCanvas  (animated node–edge background)
// ─────────────────────────────────────────────────────────────
GraphCanvas::GraphCanvas(QWidget* parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    std::srand(42);
    const int N = 28;
    m_nodes.resize(N);
    for (auto& nd : m_nodes) {
        nd.x  = (std::rand() % 1000) / 1000.0f;
        nd.y  = (std::rand() % 1000) / 1000.0f;
        nd.vx = ((std::rand() % 100) - 50) / 50000.0f;
        nd.vy = ((std::rand() % 100) - 50) / 50000.0f;
    }
    for (int i = 0; i < N; ++i)
        for (int j = i + 1; j < N; ++j) {
            float dx = m_nodes[i].x - m_nodes[j].x;
            float dy = m_nodes[i].y - m_nodes[j].y;
            if (dx*dx + dy*dy < 0.07f)
                m_edges.append({i, j});
        }

    m_timer = startTimer(16);
}

void GraphCanvas::timerEvent(QTimerEvent*)
{
    m_time += 0.003f;
    for (auto& nd : m_nodes) {
        nd.x += nd.vx;
        nd.y += nd.vy;
        if (nd.x < 0) nd.x += 1.0f;
        if (nd.x > 1) nd.x -= 1.0f;
        if (nd.y < 0) nd.y += 1.0f;
        if (nd.y > 1) nd.y -= 1.0f;
    }
    update();
}

void GraphCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int W = width(), H = height();

    for (auto& e : m_edges) {
        const auto& a = m_nodes[e.first];
        const auto& b = m_nodes[e.second];
        float dx = a.x - b.x, dy = a.y - b.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        // Slightly higher distance threshold for more connections
        float alpha = std::max(0.0f, 1.0f - dist / 0.320f);
        QColor col(110, 118, 255, int(alpha * 65)); // More opaque
        p.setPen(QPen(col, 1.8)); // Thicker lines
        p.drawLine(QPointF(a.x * W, a.y * H), QPointF(b.x * W, b.y * H));
    }

    // Nodes
    for (int i = 0; i < m_nodes.size(); ++i) {
        const auto& nd = m_nodes[i];
        float pulse = 0.5f + 0.5f * std::sin(m_time * 2.0f + i * 0.7f);
        float r = 4.0f + pulse * 3.0f; // Larger nodes (4px to 7px base)
        QColor fill(140, 148, 255, int(80 + 100 * pulse));
        p.setPen(Qt::NoPen);
        p.setBrush(fill);
        p.drawEllipse(QPointF(nd.x * W, nd.y * H), r, r);
    }
}

// ─────────────────────────────────────────────────────────────
//  AnimatedButton (For Sub-Algorithms within a Page)
// ─────────────────────────────────────────────────────────────
AnimatedButton::AnimatedButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent), m_glowColor(Palette::ACCENT_VIS)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(56);
    setMaximumHeight(56);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFont(QFont("Segoe UI", 10, QFont::Medium));
}

void AnimatedButton::setGlowOpacity(float v)
{
    m_glowOpacity = v;
    update();
}

void AnimatedButton::enterEvent(QEnterEvent* e)
{
    QPushButton::enterEvent(e);
    auto* anim = new QPropertyAnimation(this, "glowOpacity", this);
    anim->setDuration(200);
    anim->setEndValue(1.0f);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedButton::leaveEvent(QEvent* e)
{
    QPushButton::leaveEvent(e);
    auto* anim = new QPropertyAnimation(this, "glowOpacity", this);
    anim->setDuration(200);
    anim->setEndValue(0.0f);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    // Safety boundaries to prevent clipping overlap
    QRectF r = rect().adjusted(6, 6, -6, -6);
    QPainterPath path;
    path.addRoundedRect(r, 8, 8);

    if (m_glowOpacity > 0.01f) {
        QColor gc = m_glowColor;
        gc.setAlphaF(0.20f * m_glowOpacity);
        for (int i = 6; i >= 1; --i) {
            p.setPen(QPen(gc, i * 1.8f));
            p.drawPath(path);
            gc.setAlphaF(gc.alphaF() * 0.6f);
        }
    }

    QLinearGradient bg(r.topLeft(), r.bottomLeft());
    QColor c1 = m_glowColor; c1.setAlphaF(0.18f + 0.12f * m_glowOpacity);
    QColor c2 = m_glowColor; c2.setAlphaF(0.06f + 0.06f * m_glowOpacity);
    bg.setColorAt(0, c1);
    bg.setColorAt(1, c2);
    p.fillPath(path, bg);

    QColor border = m_glowColor;
    border.setAlphaF(0.35f + 0.45f * m_glowOpacity);
    p.strokePath(path, QPen(border, 1));

    p.setPen(Palette::TEXT_PRI);
    p.setFont(font());
    
    // Draw icon space if icon property is set
    QString icon = property("icon_text").toString();
    if (!icon.isEmpty()) {
        QRectF iconRect(r.x() + 16, r.y(), 32, r.height());
        p.drawText(iconRect, Qt::AlignLeft | Qt::AlignVCenter, icon);
        QRectF textRect(r.x() + 48, r.y(), r.width() - 48, r.height());
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
    } else {
        p.drawText(rect(), Qt::AlignCenter, text());
    }
}

// ─────────────────────────────────────────────────────────────
//  BigCategoryButton (Main Dashboard Category)
// ─────────────────────────────────────────────────────────────
BigCategoryButton::BigCategoryButton(const QString& title, const QString& subtitle,
                                     const QString& iconText, const QColor& accentColor,
                                     QWidget* parent)
    : QFrame(parent), m_title(title), m_subtitle(subtitle),
      m_iconText(iconText), m_accentColor(accentColor)
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(120);
}

void BigCategoryButton::enterEvent(QEnterEvent* e)
{
    QFrame::enterEvent(e);
    m_hovered = true;
    update();
}

void BigCategoryButton::leaveEvent(QEvent* e)
{
    QFrame::leaveEvent(e);
    m_hovered = false;
    update();
}

void BigCategoryButton::mouseReleaseEvent(QMouseEvent* e)
{
    QFrame::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void BigCategoryButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF full = rect();
    QPainterPath path;
    path.addRoundedRect(full.adjusted(2,2,-2,-2), 14, 14);

    QColor bg = m_hovered ? Palette::BG_CARD_H : Palette::BG_CARD;
    p.fillPath(path, bg);

    // Color bar at the top
    QLinearGradient stripGrad(full.topLeft(), full.topRight());
    QColor acc0 = m_accentColor; acc0.setAlphaF(0.0f);
    QColor acc1 = m_accentColor; acc1.setAlphaF(m_hovered ? 0.9f : 0.55f);
    stripGrad.setColorAt(0.0, acc0);
    stripGrad.setColorAt(0.5, acc1);
    stripGrad.setColorAt(1.0, acc0);
    p.fillRect(QRectF(full.x()+2, full.y()+2, full.width()-4, 3), stripGrad);

    if (m_hovered) {
        QRadialGradient glow(full.center().x(), 0, full.width() * 0.7f);
        QColor ga = m_accentColor; ga.setAlphaF(0.09f);
        QColor gb = m_accentColor; gb.setAlphaF(0.0f);
        glow.setColorAt(0, ga);
        glow.setColorAt(1, gb);
        p.fillPath(path, glow);
    }

    QColor border = m_accentColor;
    border.setAlphaF(m_hovered ? 0.45f : 0.18f);
    p.strokePath(path, QPen(border, 1));

    // Icon Circle
    QRectF iconCircle(24, full.center().y() - 28, 56, 56);
    QPainterPath circlePath;
    circlePath.addEllipse(iconCircle);
    QColor circFill = m_accentColor; circFill.setAlphaF(0.15f);
    p.fillPath(circlePath, circFill);
    QColor circBorder = m_accentColor; circBorder.setAlphaF(0.3f);
    p.strokePath(circlePath, QPen(circBorder, 1.2f));

    QFont iconFont("Segoe UI Emoji", 24);
    p.setFont(iconFont);
    p.setPen(m_accentColor);
    p.drawText(iconCircle, Qt::AlignCenter, m_iconText);

    // Text Content
    QFont titleFont("Segoe UI", 16, QFont::DemiBold);
    p.setFont(titleFont);
    p.setPen(Palette::TEXT_PRI);
    QRectF titleRect(100, full.center().y() - 24, full.width() - 130, 30);
    p.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_title);

    QFont subFont("Segoe UI", 10);
    p.setFont(subFont);
    p.setPen(Palette::TEXT_SEC);
    QRectF subRect(100, full.center().y() + 2, full.width() - 130, 24);
    p.drawText(subRect, Qt::AlignLeft | Qt::AlignVCenter, m_subtitle);
    
    // Arrow indicator
    p.setPen(QPen(m_accentColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    float ax = full.width() - 32;
    float ay = full.center().y();
    p.drawLine(QPointF(ax - 6, ay - 6), QPointF(ax, ay));
    p.drawLine(QPointF(ax - 6, ay + 6), QPointF(ax, ay));
}

// ─────────────────────────────────────────────────────────────
//  GraphVersePlatform (Main Window & Pages)
// ─────────────────────────────────────────────────────────────
GraphVersePlatform::GraphVersePlatform(QWidget* parent)
    : QMainWindow(parent)
{
    applyGlobalStyle();
    setupUi();
    resize(980, 720);
    setMinimumSize(760, 600);
    setWindowTitle("GraphVerse Platform");
}

GraphVersePlatform::~GraphVersePlatform() = default;

void GraphVersePlatform::applyGlobalStyle()
{
    qApp->setFont(QFont("Segoe UI", 10));

    setStyleSheet(R"(
        QMainWindow, QWidget {
            background: transparent;
            color: #EBEDFF;
        }
        QScrollArea {
            background: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background: #0E1120;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #2D3764;
            border-radius: 3px;
            min-height: 20px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QLabel { background: transparent; }
    )");
}

QWidget* GraphVersePlatform::buildHeader()
{
    auto* header = new QWidget;
    header->setFixedHeight(68);
    header->setStyleSheet("background: rgba(8,10,20,0.92);");

    auto* sep = new QFrame(header);
    sep->setFrameShape(QFrame::HLine);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background: rgba(45,55,100,0.8); border:none;");
    sep->setGeometry(0, 67, 9999, 1);

    auto* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(32, 0, 32, 1);

    auto* logoLabel = new QLabel("⬡");
    logoLabel->setStyleSheet("font-size:28px; color:#6366F1;");
    auto* nameLabel = new QLabel("GraphVerse");
    nameLabel->setStyleSheet("font: 700 18px 'Segoe UI'; color:#EBEDFF; letter-spacing:1px;");
    auto* tagLabel  = new QLabel("Platform");
    tagLabel->setStyleSheet("font: 400 11px 'Segoe UI'; color:#6366F1; margin-top:6px;");

    hlay->addWidget(logoLabel);
    hlay->addSpacing(8);
    hlay->addWidget(nameLabel);
    hlay->addSpacing(4);
    hlay->addWidget(tagLabel);
    hlay->addStretch();
    
    // Header navigation links for dashboard
    auto* homeLnk = new QPushButton("Dashboard");
    homeLnk->setCursor(Qt::PointingHandCursor);
    homeLnk->setStyleSheet("QPushButton { font: 500 11px 'Segoe UI'; color:#8C94BE; border:none; padding: 6px 14px; border-radius:4px; }"
                           "QPushButton:hover { color:#EBEDFF; background:rgba(99,102,241,0.15); }");
    hlay->addWidget(homeLnk);
    connect(homeLnk, &QPushButton::clicked, this, [this](){ if (m_stack) m_stack->setCurrentIndex(0); });

    return header;
}

QWidget* GraphVersePlatform::buildFooter()
{
    auto* footer = new QWidget;
    footer->setFixedHeight(44);
    footer->setStyleSheet("background: rgba(8,10,20,0.95); border-top: 1px solid rgba(45,55,100,0.5);");

    auto* lay = new QHBoxLayout(footer);
    lay->setContentsMargins(32, 0, 32, 0);

    auto* left = new QLabel("GraphVerse Platform · Custom Algorithmic Engine");
    left->setStyleSheet("font: 400 9px 'Segoe UI'; color:#50607A;");

    auto* right = new QLabel("v1.0 · Dedicated View Routing");
    right->setStyleSheet("font: 400 9px 'Segoe UI'; color:#50607A;");

    lay->addWidget(left);
    lay->addStretch();
    lay->addWidget(right);

    return footer;
}

HeroWidget* GraphVersePlatform::buildHeroSection()
{
    auto* hero = new HeroWidget;
    hero->setFixedHeight(240);

    m_canvas = new GraphCanvas(hero);
    m_canvas->setGeometry(0, 0, 1920, 240);
    hero->setCanvas(m_canvas);

    auto* overlay = new QWidget(hero);
    overlay->setGeometry(0, 0, 9999, 240);
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                           "stop:0 rgba(8,10,20,0.1), stop:1 rgba(8,10,20,0.92));");

    auto* vlay = new QVBoxLayout(hero);
    vlay->setContentsMargins(48, 48, 48, 24);
    vlay->setSpacing(8);

    auto* badge = new QLabel("  GRAPH ALGORITHM SUITE  ");
    badge->setStyleSheet(R"(
        font: 600 9px 'Segoe UI';
        color: #6366F1;
        background: rgba(99,102,241,0.12);
        border: 1px solid rgba(99,102,241,0.35);
        border-radius: 10px;
        padding: 4px 10px;
        letter-spacing: 2px;
    )");
    badge->setFixedSize(badge->sizeHint());

    auto* title = new QLabel("Explore the Universe\nof Graph Algorithms");
    title->setStyleSheet("font: 700 42px 'Segoe UI'; color: #EBEDFF; line-height: 1.15;");
    title->setAlignment(Qt::AlignCenter);

    auto* sub = new QLabel("Select a path below to discover our tools and applications.");
    sub->setStyleSheet("font: 400 12px 'Segoe UI'; color: #8C94BE;");
    sub->setAlignment(Qt::AlignCenter);

    vlay->addStretch();
    vlay->addWidget(badge, 0, Qt::AlignCenter);
    vlay->addWidget(title);
    vlay->addWidget(sub);
    vlay->addStretch();

    return hero;
}

// ─────────────────────────────────────────────────────────────
// PAGE 0: DASHBOARD / HOME
// ─────────────────────────────────────────────────────────────
QWidget* GraphVersePlatform::buildHomeView()
{
    auto* page = new QWidget;
    auto* vlay = new QVBoxLayout(page);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    vlay->addWidget(buildHeroSection());

    auto* cardsArea = new QWidget;
    auto* clay = new QVBoxLayout(cardsArea);
    clay->setContentsMargins(48, 20, 48, 60);
    clay->setSpacing(24);

    auto* lbl = new QLabel("Main Categories");
    lbl->setStyleSheet("font: 700 13px 'Segoe UI'; color:#8C94BE; letter-spacing:2px;");
    lbl->setAlignment(Qt::AlignCenter);
    clay->addWidget(lbl);

    auto* btnVis = new BigCategoryButton("Algorithm Visualisers", 
        "Explore and step through elegant algorithms like DFS, MSTs, and flows.",
        "⚡", Palette::ACCENT_VIS);
    clay->addWidget(btnVis);

    auto* btnApps = new BigCategoryButton("Real-World Applications",
        "See graphs in action solving concrete problems in translating and networks.",
        "🌐", Palette::ACCENT_APP);
    clay->addWidget(btnApps);
    clay->addStretch();

    // Wire up navigation
    connect(btnVis, &BigCategoryButton::clicked, this, [this](){
        m_stack->setCurrentIndex(1);
    });
    connect(btnApps, &BigCategoryButton::clicked, this, [this](){
        m_stack->setCurrentIndex(2);
    });

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(cardsArea);
    vlay->addWidget(scroll);
    
    return page;
}

// Helper to build identical headers for sub-pages
static QWidget* buildPageHeader(const QString& title, const QString& icon, 
                                const QColor& accent, QStackedWidget* stack)
{
    auto* header = new QWidget;
    auto* lay = new QVBoxLayout(header);
    lay->setContentsMargins(48, 36, 48, 16);
    lay->setSpacing(16);

    auto* btnBack = new AnimatedButton("← Back to Dashboard");
    btnBack->setGlowColor(QColor(140, 148, 190));
    btnBack->setMinimumHeight(44);
    btnBack->setMaximumHeight(44);
    btnBack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnBack->setMinimumWidth(200);
    QObject::connect(btnBack, &QPushButton::clicked, stack, [stack](){ stack->setCurrentIndex(0); });
    
    lay->addWidget(btnBack);
    lay->addSpacing(12);

    auto* titleLbl = new QLabel(QString("%1  %2").arg(icon, title));
    titleLbl->setStyleSheet(QString("font: 700 28px 'Segoe UI'; color: %1;").arg(accent.name()));
    lay->addWidget(titleLbl);
    
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("background: rgba(%1,%2,%3,0.3); border:none;")
        .arg(accent.red()).arg(accent.green()).arg(accent.blue()));
    sep->setFixedHeight(2);
    lay->addWidget(sep);

    return header;
}

// ─────────────────────────────────────────────────────────────
// PAGE 1: ALGORITHM VISUALISERS
// ─────────────────────────────────────────────────────────────
QWidget* GraphVersePlatform::buildVisualisersView()
{
    auto* page = new QWidget;
    auto* vlay = new QVBoxLayout(page);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    vlay->addWidget(buildPageHeader("Algorithm Visualisers", "⚡", Palette::ACCENT_VIS, m_stack));

    auto* listArea = new QWidget;
    auto* llay = new QVBoxLayout(listArea);
    llay->setContentsMargins(48, 16, 48, 60);
    llay->setSpacing(18);

    struct AlgoEntry { QString label; int stackIndex; };
    const std::vector<AlgoEntry> algos = {
        {"🌀 Maze Explorer — BFS & DFS", 4},
        {"🔗 Connected & Strongly Connected Components", 5},
        {"🌲 Spanning Trees — Kruskal · Prim · Borůvka", 6},
        {"🛤️ Shortest Paths — Dijkstra · A* · Bellman-Ford · Floyd-Warshall", 7},
        {"🌊 Flow Networks — Ford-Fulkerson & Negative Cycles", 8}
    };

    for(const auto& entry : algos) {
        auto* btn = new AnimatedButton(entry.label);
        btn->setGlowColor(Palette::ACCENT_VIS);
        btn->setMinimumHeight(64);
        btn->setMaximumHeight(64);
        btn->setFont(QFont("Segoe UI", 11, QFont::Medium));
        llay->addWidget(btn);
        int targetIdx = entry.stackIndex;
        connect(btn, &QPushButton::clicked, this, [this, targetIdx](){
            m_stack->setCurrentIndex(targetIdx);
        });
    }
    llay->addStretch();

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(listArea);
    vlay->addWidget(scroll);

    return page;
}

// ─────────────────────────────────────────────────────────────
// PAGE 2: REAL-WORLD APPS
// ─────────────────────────────────────────────────────────────
QWidget* GraphVersePlatform::buildAppsView()
{
    auto* page = new QWidget;
    auto* vlay = new QVBoxLayout(page);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    vlay->addWidget(buildPageHeader("Real-World Applications", "🌐", Palette::ACCENT_APP, m_stack));

    auto* listArea = new QWidget;
    auto* llay = new QVBoxLayout(listArea);
    llay->setContentsMargins(48, 16, 48, 60);
    llay->setSpacing(18);

    const QStringList apps = {
        "👥 Friend Suggestion System",
        "🌍 Translation Network"
    };

    for(const auto& str : apps) {
        auto* btn = new AnimatedButton(str);
        btn->setGlowColor(Palette::ACCENT_APP);
        btn->setMinimumHeight(64);
        btn->setMaximumHeight(64);
        btn->setFont(QFont("Segoe UI", 11, QFont::Medium));
        llay->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, str](){
            navigateToPlaceholder(str, 2);
        });
    }
    llay->addStretch();

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(listArea);
    vlay->addWidget(scroll);

    return page;
}

// ─────────────────────────────────────────────────────────────
// PAGE 3: PLACEHOLDER / DUMMY VIEW
// ─────────────────────────────────────────────────────────────
QWidget* GraphVersePlatform::buildPlaceholderView()
{
    m_placeholderPage = new QWidget;
    auto* vlay = new QVBoxLayout(m_placeholderPage);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    // Header area
    auto* header = new QWidget;
    auto* hlay = new QVBoxLayout(header);
    hlay->setContentsMargins(48, 36, 48, 16);
    hlay->setSpacing(16);

    m_placeholderBackBtn = new AnimatedButton("← Back to List");
    m_placeholderBackBtn->setGlowColor(QColor(140, 148, 190));
    m_placeholderBackBtn->setMinimumHeight(44);
    m_placeholderBackBtn->setMaximumHeight(44);
    m_placeholderBackBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_placeholderBackBtn->setMinimumWidth(200);
    hlay->addWidget(m_placeholderBackBtn);
    
    hlay->addSpacing(12);

    m_placeholderTitleLabel = new QLabel("Feature Name");
    m_placeholderTitleLabel->setStyleSheet("font: 700 28px 'Segoe UI'; color: #EBEDFF;");
    hlay->addWidget(m_placeholderTitleLabel);

    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: rgba(99,102,241,0.3); border:none;");
    sep->setFixedHeight(2);
    hlay->addWidget(sep);

    vlay->addWidget(header);

    // Empty content area with placeholder message
    auto* contentArea = new QWidget;
    auto* clay = new QVBoxLayout(contentArea);
    clay->setContentsMargins(48, 20, 48, 60);
    
    auto* placeholderMsg = new QLabel("This module is currently being calibrated.\nCheck back soon for the full visual experience!");
    placeholderMsg->setStyleSheet("font: 400 14px 'Segoe UI'; color: #8C94BE;");
    placeholderMsg->setAlignment(Qt::AlignCenter);
    clay->addStretch();
    clay->addWidget(placeholderMsg);
    clay->addStretch();

    vlay->addWidget(contentArea, 1);

    return m_placeholderPage;
}

void GraphVersePlatform::navigateToPlaceholder(const QString& title, int returnIndex)
{
    if (m_placeholderTitleLabel) {
        m_placeholderTitleLabel->setText(title);
    }
    if (m_placeholderBackBtn) {
        disconnect(m_placeholderBackBtn, &QPushButton::clicked, nullptr, nullptr);
        connect(m_placeholderBackBtn, &QPushButton::clicked, this, [this, returnIndex](){
            m_stack->setCurrentIndex(returnIndex);
        });
    }
    m_stack->setCurrentIndex(3);
}

// ─────────────────────────────────────────────────────────────
//  Main Setup
// ─────────────────────────────────────────────────────────────
void GraphVersePlatform::setupUi()
{
    auto* root = new QWidget(this);
    root->setStyleSheet(QString("background: rgb(%1,%2,%3);")
                        .arg(Palette::BG_DEEP.red())
                        .arg(Palette::BG_DEEP.green())
                        .arg(Palette::BG_DEEP.blue()));
    setCentralWidget(root);

    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(buildHeader());

    m_stack = new QStackedWidget;
    m_stack->addWidget(buildHomeView());          // Index 0
    m_stack->addWidget(buildVisualisersView());   // Index 1
    m_stack->addWidget(buildAppsView());          // Index 2
    m_stack->addWidget(buildPlaceholderView());   // Index 3

    // ─── Visualiser pages with Back navigation ───
    auto addVisPage = [&](QWidget* vis, int stackIdx) {
        auto* wrapper = new QWidget;
        auto* wlay = new QVBoxLayout(wrapper);
        wlay->setContentsMargins(0, 0, 0, 0);
        wlay->setSpacing(0);
        auto* backBtn = new AnimatedButton("← Back to Visualisers");
        backBtn->setGlowColor(QColor(140, 148, 190));
        backBtn->setMinimumHeight(40);
        backBtn->setMaximumHeight(40);
        backBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        backBtn->setMinimumWidth(200);
        auto* topBar = new QWidget;
        auto* tbLay = new QHBoxLayout(topBar);
        tbLay->setContentsMargins(16, 8, 16, 0);
        tbLay->addWidget(backBtn);
        tbLay->addStretch();
        wlay->addWidget(topBar);
        wlay->addWidget(vis, 1);
        QObject::connect(backBtn, &QPushButton::clicked, m_stack, [this](){ m_stack->setCurrentIndex(1); });
        return wrapper;
    };

    m_visMaze       = new VisMaze();
    m_visComponents = new VisComponents();
    m_visMST        = new VisMST();
    m_visShortest   = new VisShortest();
    m_visFlow       = new VisFlow();

    m_stack->addWidget(addVisPage(m_visMaze,       4));  // Index 4
    m_stack->addWidget(addVisPage(m_visComponents, 5));  // Index 5
    m_stack->addWidget(addVisPage(m_visMST,        6));  // Index 6
    m_stack->addWidget(addVisPage(m_visShortest,   7));  // Index 7
    m_stack->addWidget(addVisPage(m_visFlow,       8));  // Index 8

    m_stack->setCurrentIndex(0);

    rootLayout->addWidget(m_stack, 1);
    rootLayout->addWidget(buildFooter());
}
