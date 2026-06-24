#pragma once
#include <QString>

enum class ProductType {
    Unit,       // Produto unitário (custo por unidade)
    Package     // Produto em embalagem (custo da embalagem / unidades dentro)
};

struct Product {
    int         id           = 0;
    QString     name;
    QString     category;
    ProductType type         = ProductType::Unit;

    // Tipo unitário
    double      costPerUnit  = 0.0;   // Custo de compra por unidade
    double      salePrice    = 0.0;   // Preço de venda por unidade
    int         stockQty     = 0;     // Quantidade em estoque

    // Tipo embalagem
    double      packageCost  = 0.0;   // Custo da embalagem/caixa
    int         unitsInPkg   = 0;     // Unidades dentro da embalagem
    double      unitSalePrice= 0.0;   // Preço de venda por unidade (embalagem)
    int         packagesOwned= 0;     // Nº de embalagens em estoque

    // Calculados
    double profit() const {
        if (type == ProductType::Unit) {
            return (salePrice - costPerUnit) * stockQty;
        } else {
            double totalRevenue = unitsInPkg * unitSalePrice * packagesOwned;
            double totalCost    = packageCost * packagesOwned;
            return totalRevenue - totalCost;
        }
    }

    double profitPerUnit() const {
        if (type == ProductType::Unit) {
            return salePrice - costPerUnit;
        } else {
            if (unitsInPkg <= 0) return 0.0;
            return unitSalePrice - (packageCost / unitsInPkg);
        }
    }

    double profitMargin() const {
        double cost = (type == ProductType::Unit) ? costPerUnit : (unitsInPkg > 0 ? packageCost / unitsInPkg : 0);
        if (cost <= 0) return 0.0;
        return (profitPerUnit() / cost) * 100.0;
    }

    int totalUnits() const {
        if (type == ProductType::Unit) return stockQty;
        return unitsInPkg * packagesOwned;
    }
};
