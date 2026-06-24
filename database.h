#pragma once
#include <QSqlDatabase>
#include <QVector>
#include "product.h"

class Database {
public:
    static Database& instance();

    bool open(const QString& path = "caetastark.db");
    void close();
    bool isOpen() const;

    // Autenticação
    bool checkPassword(const QString& password);
    bool setPassword(const QString& password);
    bool hasPassword();

    // Produtos
    bool        addProduct(Product& p);
    bool        updateProduct(const Product& p);
    bool        deleteProduct(int id);
    QVector<Product> getAllProducts();
    QVector<Product> searchProducts(const QString& query);

    // Categorias
    QStringList getCategories();

private:
    Database() = default;
    void createTables();
    QString hashPassword(const QString& pwd);
};
