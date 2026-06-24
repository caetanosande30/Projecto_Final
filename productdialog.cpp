#include "productdialog.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

ProductDialog::ProductDialog(QWidget* parent, const Product* existing)
    : QDialog(parent)
{
    if (existing) { m_editing = true; m_editId = existing->id; }
    setWindowTitle(existing ? "Editar Produto" : "Novo Produto");
    setMinimumWidth(480);
    setModal(true);
    setupUi();
    applyStyles();
    if (existing) loadProduct(*existing);
}

void ProductDialog::setupUi() {
    auto* main = new QVBoxLayout(this);
    main->setSpacing(16);
    main->setContentsMargins(24, 24, 24, 20);

    // Título
    auto* titleLbl = new QLabel(m_editing ? "✏  Editar Produto" : "＋  Novo Produto");
    titleLbl->setObjectName("dlgTitle");
    main->addWidget(titleLbl);

    // Nome + Categoria
    auto* infoGroup = new QGroupBox("Informações Gerais");
    auto* infoForm  = new QFormLayout(infoGroup);
    infoForm->setSpacing(10);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("Nome do produto...");
    m_nameEdit->setObjectName("dlgInput");
    infoForm->addRow("Nome:", m_nameEdit);

    m_categoryCombo = new QComboBox;
    m_categoryCombo->setEditable(true);
    m_categoryCombo->setObjectName("dlgInput");
    m_categoryCombo->addItem("");
    for (const QString& c : Database::instance().getCategories())
        m_categoryCombo->addItem(c);
    infoForm->addRow("Categoria:", m_categoryCombo);
    main->addWidget(infoGroup);

    // Tipo de produto
    auto* typeGroup = new QGroupBox("Tipo de Produto");
    auto* typeLayout = new QHBoxLayout(typeGroup);
    m_unitRadio = new QRadioButton("🏷  Produto Unitário");
    m_pkgRadio  = new QRadioButton("📦  Produto em Embalagem");
    m_unitRadio->setChecked(true);
    typeLayout->addWidget(m_unitRadio);
    typeLayout->addWidget(m_pkgRadio);
    main->addWidget(typeGroup);

    // Stack de formulários
    m_stack = new QStackedWidget;

    // Página 0 — Unitário
    auto* unitPage = new QWidget;
    auto* unitForm = new QFormLayout(unitPage);
    unitForm->setSpacing(10);
    m_costEdit  = new QDoubleSpinBox; m_costEdit->setObjectName("dlgInput");
    m_costEdit->setRange(0, 9999999); m_costEdit->setDecimals(2);
    m_costEdit->setPrefix("MZN "); m_costEdit->setSingleStep(1);
    m_saleEdit  = new QDoubleSpinBox; m_saleEdit->setObjectName("dlgInput");
    m_saleEdit->setRange(0, 9999999); m_saleEdit->setDecimals(2);
    m_saleEdit->setPrefix("MZN "); m_saleEdit->setSingleStep(1);
    m_stockEdit = new QSpinBox; m_stockEdit->setObjectName("dlgInput");
    m_stockEdit->setRange(0, 9999999);
    unitForm->addRow("Custo por Unidade:", m_costEdit);
    unitForm->addRow("Preço de Venda:", m_saleEdit);
    unitForm->addRow("Qtd. em Estoque:", m_stockEdit);
    m_stack->addWidget(unitPage);

    // Página 1 — Embalagem
    auto* pkgPage = new QWidget;
    auto* pkgForm = new QFormLayout(pkgPage);
    pkgForm->setSpacing(10);
    m_pkgCostEdit    = new QDoubleSpinBox; m_pkgCostEdit->setObjectName("dlgInput");
    m_pkgCostEdit->setRange(0,9999999); m_pkgCostEdit->setDecimals(2);
    m_pkgCostEdit->setPrefix("MZN "); m_pkgCostEdit->setSingleStep(1);
    m_unitsInPkgEdit = new QSpinBox; m_unitsInPkgEdit->setObjectName("dlgInput");
    m_unitsInPkgEdit->setRange(1, 99999);
    m_unitSaleEdit   = new QDoubleSpinBox; m_unitSaleEdit->setObjectName("dlgInput");
    m_unitSaleEdit->setRange(0,9999999); m_unitSaleEdit->setDecimals(2);
    m_unitSaleEdit->setPrefix("MZN "); m_unitSaleEdit->setSingleStep(0.5);
    m_pkgsOwnedEdit  = new QSpinBox; m_pkgsOwnedEdit->setObjectName("dlgInput");
    m_pkgsOwnedEdit->setRange(0, 99999);
    pkgForm->addRow("Custo da Embalagem:", m_pkgCostEdit);
    pkgForm->addRow("Unidades na Embalagem:", m_unitsInPkgEdit);
    pkgForm->addRow("Preço de Venda/Unid.:", m_unitSaleEdit);
    pkgForm->addRow("Nº de Embalagens:", m_pkgsOwnedEdit);
    m_stack->addWidget(pkgPage);

    main->addWidget(m_stack);

    // Preview lucro
    m_profitLabel = new QLabel("Lucro estimado: —");
    m_profitLabel->setObjectName("profitPreview");
    m_profitLabel->setAlignment(Qt::AlignCenter);
    main->addWidget(m_profitLabel);

    // Botões
    auto* btnRow = new QHBoxLayout;
    auto* cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn = new QPushButton(m_editing ? "Guardar" : "Adicionar");
    m_saveBtn->setObjectName("saveBtn");
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(m_saveBtn);
    main->addLayout(btnRow);

    // Conexões
    connect(m_unitRadio, &QRadioButton::toggled, this, &ProductDialog::onTypeChanged);
    connect(m_saveBtn,   &QPushButton::clicked,  this, &ProductDialog::onSave);
    connect(cancelBtn,   &QPushButton::clicked,  this, &QDialog::reject);

    // Conectar spinboxes ao preview
    auto upd = [this]() { updateProfitPreview(); };
    connect(m_costEdit,       &QDoubleSpinBox::valueChanged, this, upd);
    connect(m_saleEdit,       &QDoubleSpinBox::valueChanged, this, upd);
    connect(m_stockEdit,      &QSpinBox::valueChanged,       this, upd);
    connect(m_pkgCostEdit,    &QDoubleSpinBox::valueChanged, this, upd);
    connect(m_unitsInPkgEdit, &QSpinBox::valueChanged,       this, upd);
    connect(m_unitSaleEdit,   &QDoubleSpinBox::valueChanged, this, upd);
    connect(m_pkgsOwnedEdit,  &QSpinBox::valueChanged,       this, upd);
}

void ProductDialog::onTypeChanged() {
    m_stack->setCurrentIndex(m_unitRadio->isChecked() ? 0 : 1);
    updateProfitPreview();
}

void ProductDialog::updateProfitPreview() {
    Product tmp = getProduct();
    double luc = tmp.profitPerUnit();
    double marg = tmp.profitMargin();
    QString color = luc >= 0 ? "#4eca82" : "#f06060";
    m_profitLabel->setText(QString(
        "<span style='color:%1'>Lucro/unid: <b>MZN %.2f</b> &nbsp;|&nbsp; Margem: <b>%.1f%%</b></span>"
    ).arg(color).arg(luc).arg(marg));
}

void ProductDialog::onSave() {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Campo obrigatório", "Por favor, insira o nome do produto.");
        return;
    }
    accept();
}

Product ProductDialog::getProduct() const {
    Product p;
    p.id       = m_editId;
    p.name     = m_nameEdit->text().trimmed();
    p.category = m_categoryCombo->currentText().trimmed();
    if (m_unitRadio->isChecked()) {
        p.type        = ProductType::Unit;
        p.costPerUnit = m_costEdit->value();
        p.salePrice   = m_saleEdit->value();
        p.stockQty    = m_stockEdit->value();
    } else {
        p.type          = ProductType::Package;
        p.packageCost   = m_pkgCostEdit->value();
        p.unitsInPkg    = m_unitsInPkgEdit->value();
        p.unitSalePrice = m_unitSaleEdit->value();
        p.packagesOwned = m_pkgsOwnedEdit->value();
    }
    return p;
}

void ProductDialog::loadProduct(const Product& p) {
    m_nameEdit->setText(p.name);
    m_categoryCombo->setCurrentText(p.category);
    if (p.type == ProductType::Unit) {
        m_unitRadio->setChecked(true);
        m_costEdit->setValue(p.costPerUnit);
        m_saleEdit->setValue(p.salePrice);
        m_stockEdit->setValue(p.stockQty);
    } else {
        m_pkgRadio->setChecked(true);
        m_pkgCostEdit->setValue(p.packageCost);
        m_unitsInPkgEdit->setValue(p.unitsInPkg);
        m_unitSaleEdit->setValue(p.unitSalePrice);
        m_pkgsOwnedEdit->setValue(p.packagesOwned);
    }
    updateProfitPreview();
}

void ProductDialog::applyStyles() {
    setStyleSheet(R"(
        QDialog {
            background: #12172a;
            color: #e0e6f0;
            font-family: 'Segoe UI';
        }
        QLabel#dlgTitle {
            font-size: 17px;
            font-weight: 700;
            color: #f0f4ff;
            padding-bottom: 4px;
        }
        QGroupBox {
            color: #6b7a9e;
            font-size: 11px;
            font-weight: 600;
            letter-spacing: 1px;
            border: 1px solid #1e2740;
            border-radius: 10px;
            margin-top: 8px;
            padding: 14px 12px 10px 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            color: #6b7a9e;
        }
        QRadioButton {
            color: #c0cce8;
            font-size: 13px;
            spacing: 8px;
        }
        QRadioButton::indicator {
            width: 16px; height: 16px;
            border-radius: 8px;
            border: 2px solid #3a4a6a;
            background: #1a2030;
        }
        QRadioButton::indicator:checked {
            background: #e8a020;
            border: 2px solid #e8a020;
        }
        QLabel { color: #a0aec8; font-size: 13px; }
        QLineEdit#dlgInput, QDoubleSpinBox#dlgInput, QSpinBox#dlgInput, QComboBox#dlgInput {
            background: #1a2030;
            border: 1px solid #2a3550;
            border-radius: 8px;
            padding: 8px 12px;
            color: #e0e6f0;
            font-size: 13px;
            min-height: 18px;
        }
        QLineEdit#dlgInput:focus, QDoubleSpinBox#dlgInput:focus,
        QSpinBox#dlgInput:focus, QComboBox#dlgInput:focus {
            border: 1.5px solid #e8a020;
        }
        QDoubleSpinBox#dlgInput::up-button, QSpinBox#dlgInput::up-button,
        QDoubleSpinBox#dlgInput::down-button, QSpinBox#dlgInput::down-button {
            background: #2a3550; border: none; width: 18px;
        }
        QComboBox#dlgInput::drop-down { border: none; background: #2a3550; width:24px; border-radius:0 8px 8px 0; }
        QComboBox QAbstractItemView { background:#1a2030; color:#e0e6f0; selection-background-color:#e8a020; }
        QLabel#profitPreview {
            background: #1a2030;
            border: 1px solid #2a3550;
            border-radius: 8px;
            padding: 10px;
            font-size: 13px;
        }
        QPushButton#saveBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #e8a020,stop:1 #c97d10);
            border: none; border-radius: 8px; padding: 11px 24px;
            font-weight: 700; font-size: 13px; color: #0f1219; letter-spacing: 1px;
        }
        QPushButton#saveBtn:hover { background: #f0b030; }
        QPushButton#cancelBtn {
            background: #1e2740; border: 1px solid #2a3550;
            border-radius: 8px; padding: 11px 24px;
            font-size: 13px; color: #8090b0;
        }
        QPushButton#cancelBtn:hover { background: #252e48; color: #c0cce8; }
    )");
}
