#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "product.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onAddProduct();
    void onEditProduct();
    void onDeleteProduct();
    void onSearch(const QString& text);
    void onFilterCategory(const QString& cat);
    void onExportPDF();
    void onExportCSV();
    void refreshTable();
    void updateSummaryCards();

private:
    void setupUi();
    void applyStyles();
    void setupTable();
    void populateTable(const QVector<Product>& products);
    void refreshCategories();

    QTableWidget*  m_table         = nullptr;
    QLineEdit*     m_searchEdit    = nullptr;
    QComboBox*     m_catFilter     = nullptr;
    QLabel*        m_cardTotal     = nullptr;
    QLabel*        m_cardProfit    = nullptr;
    QLabel*        m_cardProducts  = nullptr;
    QPushButton*   m_editBtn       = nullptr;
    QPushButton*   m_deleteBtn     = nullptr;

    QVector<Product> m_products;
};
