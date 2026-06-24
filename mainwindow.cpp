#include "mainwindow.h"
#include "database.h"
#include "productdialog.h"
#include "exportmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMenuBar>
#include <QStatusBar>
#include <QFrame>
#include <QFont>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Caeta Stark — Gestão de Estoque");
    setMinimumSize(1000, 650);
    resize(1200, 720);
    setupUi();
    applyStyles();
    refreshTable();
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* root = new QVBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Header ──────────────────────────────────────────────
    auto* header = new QWidget;
    header->setObjectName("appHeader");
    header->setFixedHeight(64);
    auto* hdrLayout = new QHBoxLayout(header);
    hdrLayout->setContentsMargins(28, 0, 24, 0);

    auto* logoArea = new QHBoxLayout;
    auto* hexLbl   = new QLabel("⬡");
    hexLbl->setObjectName("headerHex");
    auto* titleLbl = new QLabel("CAETA STARK");
    titleLbl->setObjectName("headerTitle");
    auto* tagLbl   = new QLabel("· Gestão de Estoque");
    tagLbl->setObjectName("headerTag");
    logoArea->addWidget(hexLbl);
    logoArea->addWidget(titleLbl);
    logoArea->addWidget(tagLbl);
    logoArea->addStretch();
    hdrLayout->addLayout(logoArea);
    root->addWidget(header);

    // ── Summary Cards ────────────────────────────────────────
    auto* cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(16);
    cardsRow->setContentsMargins(24, 20, 24, 4);

    auto makeCard = [&](const QString& icon, const QString& title, QLabel*& valueLabel) {
        auto* card = new QFrame;
        card->setObjectName("summaryCard");
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 14, 20, 14);
        cl->setSpacing(4);
        auto* top = new QHBoxLayout;
        auto* ico = new QLabel(icon); ico->setObjectName("cardIcon");
        auto* ttl = new QLabel(title); ttl->setObjectName("cardTitle");
        top->addWidget(ico); top->addWidget(ttl); top->addStretch();
        valueLabel = new QLabel("—"); valueLabel->setObjectName("cardValue");
        cl->addLayout(top);
        cl->addWidget(valueLabel);
        return card;
    };

    cardsRow->addWidget(makeCard("📦", "TOTAL DE PRODUTOS", m_cardProducts));
    cardsRow->addWidget(makeCard("📊", "LUCRO POTENCIAL TOTAL", m_cardProfit));
    cardsRow->addWidget(makeCard("🏷", "TOTAL EM ESTOQUE", m_cardTotal));
    root->addLayout(cardsRow);

    // ── Toolbar ──────────────────────────────────────────────
    auto* toolbar = new QWidget;
    toolbar->setObjectName("toolbarWidget");
    auto* tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(24, 12, 24, 8);
    tbLayout->setSpacing(10);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("🔍  Pesquisar produto...");
    m_searchEdit->setObjectName("searchInput");
    m_searchEdit->setFixedWidth(260);

    m_catFilter = new QComboBox;
    m_catFilter->setObjectName("filterCombo");
    m_catFilter->addItem("Todas as categorias");
    m_catFilter->setFixedWidth(180);

    tbLayout->addWidget(m_searchEdit);
    tbLayout->addWidget(m_catFilter);
    tbLayout->addStretch();

    auto mkBtn = [&](const QString& label, const QString& objName) {
        auto* btn = new QPushButton(label);
        btn->setObjectName(objName);
        btn->setCursor(Qt::PointingHandCursor);
        return btn;
    };

    auto* addBtn    = mkBtn("＋  Adicionar",   "addBtn");
    m_editBtn       = mkBtn("✏  Editar",       "editBtn");
    m_deleteBtn     = mkBtn("🗑  Eliminar",     "deleteBtn");
    auto* exportPDF = mkBtn("⬇  PDF",          "exportBtn");
    auto* exportCSV = mkBtn("⬇  Excel/CSV",    "exportBtn");

    tbLayout->addWidget(addBtn);
    tbLayout->addWidget(m_editBtn);
    tbLayout->addWidget(m_deleteBtn);
    tbLayout->addWidget(exportPDF);
    tbLayout->addWidget(exportCSV);
    root->addWidget(toolbar);

    // ── Tabela ───────────────────────────────────────────────
    auto* tableWrap = new QWidget;
    tableWrap->setObjectName("tableWrap");
    auto* twl = new QVBoxLayout(tableWrap);
    twl->setContentsMargins(24, 4, 24, 16);

    m_table = new QTableWidget;
    m_table->setObjectName("productTable");
    setupTable();
    twl->addWidget(m_table);
    root->addWidget(tableWrap);

    // Statusbar
    statusBar()->setObjectName("appStatus");
    statusBar()->showMessage("Bem-vindo ao Caeta Stark");

    // Conexões
    connect(addBtn,     &QPushButton::clicked,         this, &MainWindow::onAddProduct);
    connect(m_editBtn,  &QPushButton::clicked,         this, &MainWindow::onEditProduct);
    connect(m_deleteBtn,&QPushButton::clicked,         this, &MainWindow::onDeleteProduct);
    connect(exportPDF,  &QPushButton::clicked,         this, &MainWindow::onExportPDF);
    connect(exportCSV,  &QPushButton::clicked,         this, &MainWindow::onExportCSV);
    connect(m_searchEdit,&QLineEdit::textChanged,      this, &MainWindow::onSearch);
    connect(m_catFilter, &QComboBox::currentTextChanged,this,&MainWindow::onFilterCategory);
    connect(m_table,    &QTableWidget::itemDoubleClicked,this,&MainWindow::onEditProduct);
}

void MainWindow::setupTable() {
    QStringList headers = {
        "Nome", "Tipo", "Categoria",
        "Custo", "Venda", "Unid. em Estoque",
        "Lucro/Unid.", "Lucro Total", "Margem %"
    };
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
}

void MainWindow::populateTable(const QVector<Product>& products) {
    m_table->setSortingEnabled(false);
    m_table->setRowCount(products.size());

    for (int i = 0; i < products.size(); ++i) {
        const Product& p = products[i];
        bool isUnit = (p.type == ProductType::Unit);
        double cost = isUnit ? p.costPerUnit : (p.unitsInPkg > 0 ? p.packageCost / p.unitsInPkg : 0);
        double sale = isUnit ? p.salePrice : p.unitSalePrice;

        auto item = [&](const QString& txt, Qt::Alignment align = Qt::AlignVCenter | Qt::AlignLeft) {
            auto* it = new QTableWidgetItem(txt);
            it->setTextAlignment(align);
            return it;
        };
        auto numItem = [&](double val, const QString& prefix = "MZN ") {
            auto* it = new QTableWidgetItem(prefix + QString::number(val, 'f', 2));
            it->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            it->setData(Qt::UserRole, val);
            return it;
        };

        m_table->setItem(i, 0, item(p.name));

        auto* typeItem = item(isUnit ? "Unitário" : "Embalagem", Qt::AlignCenter);
        typeItem->setForeground(isUnit ? QColor("#70b8f8") : QColor("#f8c060"));
        m_table->setItem(i, 1, typeItem);

        m_table->setItem(i, 2, item(p.category.isEmpty() ? "—" : p.category, Qt::AlignCenter));
        m_table->setItem(i, 3, numItem(cost));
        m_table->setItem(i, 4, numItem(sale));

        auto* unitsItem = new QTableWidgetItem(QString::number(p.totalUnits()));
        unitsItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 5, unitsItem);

        auto* lucItem = numItem(p.profitPerUnit());
        lucItem->setForeground(p.profitPerUnit() >= 0 ? QColor("#4eca82") : QColor("#f06060"));
        m_table->setItem(i, 6, lucItem);

        auto* totItem = numItem(p.profit());
        totItem->setForeground(p.profit() >= 0 ? QColor("#4eca82") : QColor("#f06060"));
        m_table->setItem(i, 7, totItem);

        auto* margItem = new QTableWidgetItem(QString::number(p.profitMargin(), 'f', 1) + "%");
        margItem->setTextAlignment(Qt::AlignCenter);
        margItem->setForeground(p.profitMargin() >= 0 ? QColor("#4eca82") : QColor("#f06060"));
        m_table->setItem(i, 8, margItem);

        // Guardar ID na linha
        m_table->item(i, 0)->setData(Qt::UserRole, p.id);
        m_table->setRowHeight(i, 38);
    }
    m_table->setSortingEnabled(true);
}

void MainWindow::refreshTable() {
    m_products = Database::instance().getAllProducts();
    populateTable(m_products);
    updateSummaryCards();
    refreshCategories();
    statusBar()->showMessage(QString("Total: %1 produto(s)").arg(m_products.size()));
}

void MainWindow::updateSummaryCards() {
    double totalProfit = 0.0;
    int    totalUnits  = 0;
    for (const Product& p : m_products) {
        totalProfit += p.profit();
        totalUnits  += p.totalUnits();
    }
    m_cardProducts->setText(QString::number(m_products.size()));
    m_cardTotal->setText(QString::number(totalUnits) + " unidades");
    m_cardProfit->setText(
        "<span style='color:" +
        QString(totalProfit >= 0 ? "#4eca82" : "#f06060") +
        "'>MZN " + QString::number(totalProfit, 'f', 2) + "</span>"
    );
}

void MainWindow::refreshCategories() {
    QString current = m_catFilter->currentText();
    m_catFilter->blockSignals(true);
    m_catFilter->clear();
    m_catFilter->addItem("Todas as categorias");
    for (const QString& c : Database::instance().getCategories())
        m_catFilter->addItem(c);
    m_catFilter->setCurrentText(current);
    m_catFilter->blockSignals(false);
}

void MainWindow::onAddProduct() {
    ProductDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Product p = dlg.getProduct();
        Database::instance().addProduct(p);
        refreshTable();
        statusBar()->showMessage("Produto adicionado com sucesso.", 3000);
    }
}

void MainWindow::onEditProduct() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::information(this, "Editar", "Seleccione um produto primeiro."); return; }
    int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    Product* found = nullptr;
    for (Product& p : m_products) { if (p.id == id) { found = &p; break; } }
    if (!found) return;
    ProductDialog dlg(this, found);
    if (dlg.exec() == QDialog::Accepted) {
        Product updated = dlg.getProduct();
        updated.id = id;
        Database::instance().updateProduct(updated);
        refreshTable();
        statusBar()->showMessage("Produto actualizado.", 3000);
    }
}

void MainWindow::onDeleteProduct() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::information(this, "Eliminar", "Seleccione um produto primeiro."); return; }
    QString name = m_table->item(row, 0)->text();
    auto reply = QMessageBox::question(this, "Eliminar Produto",
        QString("Tem a certeza que deseja eliminar <b>%1</b>?").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
        Database::instance().deleteProduct(id);
        refreshTable();
        statusBar()->showMessage("Produto eliminado.", 3000);
    }
}

void MainWindow::onSearch(const QString& text) {
    if (text.isEmpty()) { populateTable(m_products); return; }
    QVector<Product> results;
    for (const Product& p : m_products)
        if (p.name.contains(text, Qt::CaseInsensitive) ||
            p.category.contains(text, Qt::CaseInsensitive))
            results.append(p);
    populateTable(results);
}

void MainWindow::onFilterCategory(const QString& cat) {
    if (cat == "Todas as categorias") { populateTable(m_products); return; }
    QVector<Product> filtered;
    for (const Product& p : m_products)
        if (p.category == cat) filtered.append(p);
    populateTable(filtered);
}

void MainWindow::onExportPDF() {
    QString path = QFileDialog::getSaveFileName(this,
        "Exportar para PDF", "relatorio_estoque.pdf", "PDF (*.pdf)");
    if (path.isEmpty()) return;
    if (ExportManager::exportToPDF(m_products, path))
        QMessageBox::information(this, "Exportação", "Relatório PDF gerado com sucesso!");
    else
        QMessageBox::critical(this, "Erro", "Não foi possível gerar o PDF.");
}

void MainWindow::onExportCSV() {
    QString path = QFileDialog::getSaveFileName(this,
        "Exportar para CSV", "estoque.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;
    if (ExportManager::exportToCSV(m_products, path))
        QMessageBox::information(this, "Exportação", "Ficheiro CSV gerado com sucesso!\nPode abrir no Excel.");
    else
        QMessageBox::critical(this, "Erro", "Não foi possível criar o ficheiro.");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    auto reply = QMessageBox::question(this, "Sair",
        "<b>Deseja guardar os dados antes de sair?</b><br>"
        "<small>Os dados já estão guardados automaticamente na base de dados.</small>",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Cancel) { event->ignore(); return; }
    if (reply == QMessageBox::Yes) {
        // Dados já persistidos no SQLite — confirmação visual
        statusBar()->showMessage("Dados guardados.");
    }
    Database::instance().close();
    event->accept();
}

void MainWindow::applyStyles() {
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background: #0e1120;
            color: #d0d8f0;
            font-family: 'Segoe UI';
        }
        QWidget#appHeader {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #141828, stop:1 #0e1120);
            border-bottom: 2px solid #e8a020;
        }
        QLabel#headerHex   { font-size: 26px; color: #e8a020; }
        QLabel#headerTitle {
            font-size: 19px; font-weight: 800;
            letter-spacing: 5px; color: #f0f4ff;
        }
        QLabel#headerTag   { font-size: 12px; color: #4a5878; }

        QFrame#summaryCard {
            background: #141828;
            border: 1px solid #1e2740;
            border-radius: 12px;
            min-height: 80px;
        }
        QLabel#cardIcon  { font-size: 18px; }
        QLabel#cardTitle {
            font-size: 9px; font-weight: 700;
            letter-spacing: 2px; color: #5a6888;
        }
        QLabel#cardValue {
            font-size: 22px; font-weight: 700;
            color: #f0f4ff;
        }

        QWidget#toolbarWidget { background: #0e1120; }
        QLineEdit#searchInput {
            background: #141828;
            border: 1px solid #1e2740;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 13px;
            color: #d0d8f0;
        }
        QLineEdit#searchInput:focus { border: 1.5px solid #e8a020; }
        QComboBox#filterCombo {
            background: #141828;
            border: 1px solid #1e2740;
            border-radius: 8px;
            padding: 8px 14px;
            font-size: 12px;
            color: #a0aec8;
        }
        QComboBox#filterCombo::drop-down { border:none; background:#1e2740; width:22px; border-radius:0 8px 8px 0; }
        QComboBox QAbstractItemView { background:#141828; color:#d0d8f0; selection-background-color:#e8a020; }

        QPushButton#addBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #e8a020,stop:1 #c97d10);
            border:none; border-radius:8px; padding:9px 18px;
            font-weight:700; font-size:12px; color:#0f1219; letter-spacing:1px;
        }
        QPushButton#addBtn:hover { background:#f0b030; }
        QPushButton#editBtn, QPushButton#deleteBtn, QPushButton#exportBtn {
            background: #141828;
            border: 1px solid #1e2740;
            border-radius: 8px; padding: 9px 16px;
            font-size: 12px; color: #8090b0;
        }
        QPushButton#editBtn:hover   { border-color:#4080c0; color:#80b8e8; }
        QPushButton#deleteBtn:hover { border-color:#c04040; color:#f08080; }
        QPushButton#exportBtn:hover { border-color:#40a060; color:#80d0a0; }

        QWidget#tableWrap { background: #0e1120; }
        QTableWidget#productTable {
            background: #0e1120;
            alternate-background-color: #121626;
            border: 1px solid #1a2035;
            border-radius: 10px;
            gridline-color: transparent;
            selection-background-color: #1e3a5f;
            font-size: 13px;
            color: #c0cce8;
        }
        QHeaderView::section {
            background: #141828;
            color: #e8a020;
            font-weight: 700;
            font-size: 10px;
            letter-spacing: 1px;
            padding: 10px 12px;
            border: none;
            border-bottom: 2px solid #e8a020;
        }
        QScrollBar:vertical {
            background: #0e1120; width: 8px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #2a3550; border-radius: 4px; min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #e8a020; }
        QScrollBar::add-line, QScrollBar::sub-line { height: 0; }

        QStatusBar#appStatus {
            background: #0a0d18;
            color: #4a5878;
            font-size: 11px;
            border-top: 1px solid #1a2035;
            padding: 4px 20px;
        }

        QMessageBox {
            background: #141828;
            color: #d0d8f0;
            font-family: 'Segoe UI';
        }
        QMessageBox QLabel { color: #d0d8f0; }
        QMessageBox QPushButton {
            background: #1e2740; border: 1px solid #2a3550;
            border-radius:6px; padding:7px 20px; color:#c0cce8; min-width:80px;
        }
        QMessageBox QPushButton:default {
            background: #c97d10; color: #0f1219; border: none;
        }
    )");
}
