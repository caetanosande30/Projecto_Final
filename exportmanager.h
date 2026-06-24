#pragma once
#include <QVector>
#include <QString>
#include "product.h"

class ExportManager {
public:
    static bool exportToPDF(const QVector<Product>& products, const QString& filePath);
    static bool exportToCSV(const QVector<Product>& products, const QString& filePath);
};
