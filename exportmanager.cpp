#include "exportmanager.h"
#include <QTextDocument>
#include <QPrinter>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

bool ExportManager::exportToPDF(const QVector<Product>& products, const QString& filePath) {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QString html;
    html += R"(
    <html><head><style>
      body  { font-family: Arial, sans-serif; font-size: 11pt; color: #1a1a2e; margin: 20px; }
      h1    { font-size: 20pt; color: #b87010; letter-spacing: 3px; margin-bottom: 2px; }
      p.sub { font-size: 9pt; color: #666; margin-top: 0; }
      table { width: 100%; border-collapse: collapse; margin-top: 16px; }
      th    { background: #1a1f2e; color: #e8a020; padding: 8px 10px;
              font-size: 9pt; letter-spacing: 1px; text-align: left; }
      td    { padding: 7px 10px; font-size: 10pt; border-bottom: 1px solid #eee; }
      tr:nth-child(even) td { background: #f8f9fc; }
      .green { color: #1a9a5a; font-weight: bold; }
      .red   { color: #c03030; font-weight: bold; }
      .badge-unit { background:#e8f4ff; color:#1060b0; padding:2px 7px; border-radius:4px; font-size:8pt; }
      .badge-pkg  { background:#fff4e0; color:#a06000; padding:2px 7px; border-radius:4px; font-size:8pt; }
      .totals { margin-top: 20px; padding: 12px; background: #f0f4ff;
                border-left: 4px solid #e8a020; font-size: 11pt; }
      .totals b { color: #b87010; }
    </style></head><body>
    )";

    html += "<h1>CAETA STARK</h1>";
    html += "<p class='sub'>Relat&oacute;rio de Estoque &nbsp;&middot;&nbsp; ";
    html += QDateTime::currentDateTime().toString("dd/MM/yyyy  hh:mm");
    html += "</p><hr/>";

    html += "<table><tr>"
            "<th>Produto</th>"
            "<th>Tipo</th>"
            "<th>Categoria</th>"
            "<th>Custo</th>"
            "<th>Venda</th>"
            "<th>Estoque</th>"
            "<th>Lucro/Unid.</th>"
            "<th>Lucro Total</th>"
            "</tr>";

    double totalProfit = 0.0;
    int    totalUnits  = 0;

    for (const Product& p : products) {
        bool   isUnit = (p.type == ProductType::Unit);
        double cost   = isUnit ? p.costPerUnit
                               : (p.unitsInPkg > 0 ? p.packageCost / p.unitsInPkg : 0.0);
        double sale   = isUnit ? p.salePrice : p.unitSalePrice;
        int    units  = p.totalUnits();

        totalProfit += p.profit();
        totalUnits  += units;

        QString lucColor = p.profitPerUnit() >= 0 ? "green" : "red";
        QString totColor = p.profit()        >= 0 ? "green" : "red";
        QString badge    = isUnit
            ? "<span class='badge-unit'>Unit&aacute;rio</span>"
            : "<span class='badge-pkg'>Embalagem</span>";
        QString cat = p.category.isEmpty() ? "&mdash;" : p.category.toHtmlEscaped();

        html += "<tr>"
                "<td>" + p.name.toHtmlEscaped() + "</td>"
                "<td>" + badge + "</td>"
                "<td>" + cat + "</td>"
                "<td>MZN " + QString::number(cost, 'f', 2) + "</td>"
                "<td>MZN " + QString::number(sale, 'f', 2) + "</td>"
                "<td style='text-align:center'>" + QString::number(units) + "</td>"
                "<td class='" + lucColor + "'>MZN " + QString::number(p.profitPerUnit(), 'f', 2) + "</td>"
                "<td class='" + totColor + "'>MZN " + QString::number(p.profit(), 'f', 2) + "</td>"
                "</tr>";
    }

    html += "</table>";

    QString profitColor = totalProfit >= 0 ? "#1a9a5a" : "#c03030";
    html += "<div class='totals'>"
            "<b>Resumo:</b> &nbsp;"
            "Total de produtos: <b>" + QString::number(products.size()) + "</b> &nbsp;|&nbsp; "
            "Total de unidades: <b>" + QString::number(totalUnits) + "</b> &nbsp;|&nbsp; "
            "Lucro total potencial: <b style='color:" + profitColor + "'>MZN " +
            QString::number(totalProfit, 'f', 2) + "</b>"
            "</div>";

    html += "</body></html>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);

    return true;
}

bool ExportManager::exportToCSV(const QVector<Product>& products, const QString& filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);

    // BOM para Excel reconhecer UTF-8
    out << "\xEF\xBB\xBF";
    out << "Nome,Tipo,Categoria,"
           "Custo (MZN),Preco Venda (MZN),Estoque (Unidades),"
           "Lucro por Unidade (MZN),Lucro Total (MZN),Margem (%)\n";

    for (const Product& p : products) {
        bool   isUnit = (p.type == ProductType::Unit);
        double cost   = isUnit ? p.costPerUnit
                               : (p.unitsInPkg > 0 ? p.packageCost / p.unitsInPkg : 0.0);
        double sale   = isUnit ? p.salePrice : p.unitSalePrice;

        out << "\"" << p.name                                   << "\","
            << (isUnit ? "Unitario" : "Embalagem")              << ","
            << "\"" << p.category                               << "\","
            << QString::number(cost,              'f', 2)       << ","
            << QString::number(sale,              'f', 2)       << ","
            << QString::number(p.totalUnits())                  << ","
            << QString::number(p.profitPerUnit(), 'f', 2)       << ","
            << QString::number(p.profit(),        'f', 2)       << ","
            << QString::number(p.profitMargin(),  'f', 1)       << "\n";
    }

    f.close();
    return true;
}
