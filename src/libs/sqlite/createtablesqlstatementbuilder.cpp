/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "createtablesqlstatementbuilder.h"

namespace Sqlite {

CreateTableSqlStatementBuilder::CreateTableSqlStatementBuilder()
    : m_sqlStatementBuilder("CREATE TABLE IF NOT EXISTS $table($columnDefinitions)$withoutRowId"),
      m_useWithoutRowId(false)
{
}

void CreateTableSqlStatementBuilder::setTable(Utils::SmallString &&tableName)
{
    m_sqlStatementBuilder.clear();

    this->m_tableName = std::move(tableName);
}

void CreateTableSqlStatementBuilder::addColumn(Utils::SmallString &&columnName,
                                               ColumnType columnType,
                                               IsPrimaryKey isPrimaryKey)
{
    m_sqlStatementBuilder.clear();

    m_columns.emplace_back(std::move(columnName), columnType, isPrimaryKey);
}

void CreateTableSqlStatementBuilder::setColumns(const SqliteColumns &columns)
{
    m_sqlStatementBuilder.clear();

    m_columns = std::move(columns);
}

void CreateTableSqlStatementBuilder::setUseWithoutRowId(bool useWithoutRowId)
{
    m_useWithoutRowId = useWithoutRowId;
}

void CreateTableSqlStatementBuilder::clear()
{
    m_sqlStatementBuilder.clear();
    m_columns.clear();
    m_tableName.clear();
    m_useWithoutRowId = false;
}

void CreateTableSqlStatementBuilder::clearColumns()
{
    m_sqlStatementBuilder.clear();
    m_columns.clear();
}

Utils::SmallStringView CreateTableSqlStatementBuilder::sqlStatement() const
{
    if (!m_sqlStatementBuilder.isBuild())
        bindAll();

    return m_sqlStatementBuilder.sqlStatement();
}

bool CreateTableSqlStatementBuilder::isValid() const
{
    return m_tableName.hasContent() && !m_columns.empty();
}

void CreateTableSqlStatementBuilder::bindColumnDefinitions() const
{
    Utils::SmallStringVector columnDefinitionStrings;

    for (const SqliteColumn &columns : m_columns) {
        Utils::SmallString columnDefinitionString = {columns.name(), " ", columns.typeString()};

        if (columns.isPrimaryKey())
            columnDefinitionString.append(" PRIMARY KEY");

        columnDefinitionStrings.push_back(columnDefinitionString);
    }

    m_sqlStatementBuilder.bind("$columnDefinitions", columnDefinitionStrings);
}

void CreateTableSqlStatementBuilder::bindAll() const
{
    m_sqlStatementBuilder.bind("$table", m_tableName.clone());

    bindColumnDefinitions();

    if (m_useWithoutRowId)
        m_sqlStatementBuilder.bind("$withoutRowId", " WITHOUT ROWID");
    else
        m_sqlStatementBuilder.bindEmptyText("$withoutRowId");
}

} // namespace Sqlite
