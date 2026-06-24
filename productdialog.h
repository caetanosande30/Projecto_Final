#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include "product.h"

class ProductDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProductDialog(QWidget* parent = nullptr, const Product* existing = nullptr);
    Product getProduct() const;

private slots:
    void onTypeChanged();
    void updateProfitPreview();
    void onSave();

private:
    void setupUi();
    void applyStyles();
    void loadProduct(const Product& p);

    // Campos comuns
    QLineEdit*      m_nameEdit      = nullptr;
    QComboBox*      m_categoryCombo = nullptr;
    QRadioButton*   m_unitRadio     = nullptr;
    QRadioButton*   m_pkgRadio      = nullptr;
    QStackedWidget* m_stack         = nullptr;

    // Tipo unitário
    QDoubleSpinBox* m_costEdit      = nullptr;
    QDoubleSpinBox* m_saleEdit      = nullptr;
    QSpinBox*       m_stockEdit     = nullptr;

    // Tipo embalagem
    QDoubleSpinBox* m_pkgCostEdit   = nullptr;
    QSpinBox*       m_unitsInPkgEdit= nullptr;
    QDoubleSpinBox* m_unitSaleEdit  = nullptr;
    QSpinBox*       m_pkgsOwnedEdit = nullptr;

    // Preview lucro
    QLabel*         m_profitLabel   = nullptr;
    QPushButton*    m_saveBtn       = nullptr;

    bool m_editing = false;
    int  m_editId  = 0;
};
