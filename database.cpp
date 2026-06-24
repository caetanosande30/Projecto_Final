#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QVariant>

Database& Database::instance() {
    static Database db;
    return db;
}

bool Database::open(const QString& path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        qDebug() << "DB Error:" << db.lastError().text();
        return false;
    }
    createTables();
    return true;
}

void Database::close() {
    QSqlDatabase::database().close();
}

bool Database::isOpen() const {
    return QSqlDatabase::database().isOpen();
}

void Database::createTables() {
    QSqlQuery q;
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS settings (
            key   TEXT PRIMARY KEY,
            value TEXT
        )
    )");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS products (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            name           TEXT    NOT NULL,
            category       TEXT    DEFAULT '',
            type           INTEGER DEFAULT 0,
            cost_per_unit  REAL    DEFAULT 0,
            sale_price     REAL    DEFAULT 0,
            stock_qty      INTEGER DEFAULT 0,
            package_cost   REAL    DEFAULT 0,
            units_in_pkg   INTEGER DEFAULT 0,
            unit_sale_price REAL   DEFAULT 0,
            packages_owned INTEGER DEFAULT 0
        )
    )");
}

QString Database::hashPassword(const QString& pwd) {
    return QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool Database::hasPassword() {
    QSqlQuery q("SELECT value FROM settings WHERE key='password'");
    return q.next() && !q.value(0).toString().isEmpty();
}

bool Database::checkPassword(const QString& password) {
    QSqlQuery q;
    q.prepare("SELECT value FROM settings WHERE key='password'");
    q.exec();
    if (!q.next()) return false;
    return q.value(0).toString() == hashPassword(password);
}

bool Database::setPassword(const QString& password) {
    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES ('password', :v)");
    q.bindValue(":v", hashPassword(password));
    return q.exec();
}

bool Database::addProduct(Product& p) {
    QSqlQuery q;
    q.prepare(R"(
        INSERT INTO products
        (name, category, type, cost_per_unit, sale_price, stock_qty,
         package_cost, units_in_pkg, unit_sale_price, packages_owned)
        VALUES (:name,:cat,:type,:cpu,:sp,:sq,:pc,:uip,:usp,:po)
    )");
    q.bindValue(":name", p.name);
    q.bindValue(":cat",  p.category);
    q.bindValue(":type", static_cast<int>(p.type));
    q.bindValue(":cpu",  p.costPerUnit);
    q.bindValue(":sp",   p.salePrice);
    q.bindValue(":sq",   p.stockQty);
    q.bindValue(":pc",   p.packageCost);
    q.bindValue(":uip",  p.unitsInPkg);
    q.bindValue(":usp",  p.unitSalePrice);
    q.bindValue(":po",   p.packagesOwned);
    if (!q.exec()) { qDebug() << q.lastError(); return false; }
    p.id = q.lastInsertId().toInt();
    return true;
}

bool Database::updateProduct(const Product& p) {
    QSqlQuery q;
    q.prepare(R"(
        UPDATE products SET
        name=:name, category=:cat, type=:type, cost_per_unit=:cpu,
        sale_price=:sp, stock_qty=:sq, package_cost=:pc,
        units_in_pkg=:uip, unit_sale_price=:usp, packages_owned=:po
        WHERE id=:id
    )");
    q.bindValue(":name", p.name);
    q.bindValue(":cat",  p.category);
    q.bindValue(":type", static_cast<int>(p.type));
    q.bindValue(":cpu",  p.costPerUnit);
    q.bindValue(":sp",   p.salePrice);
    q.bindValue(":sq",   p.stockQty);
    q.bindValue(":pc",   p.packageCost);
    q.bindValue(":uip",  p.unitsInPkg);
    q.bindValue(":usp",  p.unitSalePrice);
    q.bindValue(":po",   p.packagesOwned);
    q.bindValue(":id",   p.id);
    return q.exec();
}

bool Database::deleteProduct(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM products WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}

QVector<Product> Database::getAllProducts() {
    QVector<Product> list;
    QSqlQuery q("SELECT * FROM products ORDER BY name");
    while (q.next()) {
        Product p;
        p.id            = q.value("id").toInt();
        p.name          = q.value("name").toString();
        p.category      = q.value("category").toString();
        p.type          = static_cast<ProductType>(q.value("type").toInt());
        p.costPerUnit   = q.value("cost_per_unit").toDouble();
        p.salePrice     = q.value("sale_price").toDouble();
        p.stockQty      = q.value("stock_qty").toInt();
        p.packageCost   = q.value("package_cost").toDouble();
        p.unitsInPkg    = q.value("units_in_pkg").toInt();
        p.unitSalePrice = q.value("unit_sale_price").toDouble();
        p.packagesOwned = q.value("packages_owned").toInt();
        list.append(p);
    }
    return list;
}

QVector<Product> Database::searchProducts(const QString& query) {
    QVector<Product> list;
    QSqlQuery q;
    q.prepare("SELECT * FROM products WHERE name LIKE :q OR category LIKE :q ORDER BY name");
    q.bindValue(":q", "%" + query + "%");
    q.exec();
    while (q.next()) {
        Product p;
        p.id            = q.value("id").toInt();
        p.name          = q.value("name").toString();
        p.category      = q.value("category").toString();
        p.type          = static_cast<ProductType>(q.value("type").toInt());
        p.costPerUnit   = q.value("cost_per_unit").toDouble();
        p.salePrice     = q.value("sale_price").toDouble();
        p.stockQty      = q.value("stock_qty").toInt();
        p.packageCost   = q.value("package_cost").toDouble();
        p.unitsInPkg    = q.value("units_in_pkg").toInt();
        p.unitSalePrice = q.value("unit_sale_price").toDouble();
        p.packagesOwned = q.value("packages_owned").toInt();
        list.append(p);
    }
    return list;
}

QStringList Database::getCategories() {
    QStringList cats;
    QSqlQuery q("SELECT DISTINCT category FROM products WHERE category != '' ORDER BY category");
    while (q.next()) cats << q.value(0).toString();
    return cats;
}
