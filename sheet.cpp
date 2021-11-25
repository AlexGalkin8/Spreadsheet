#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>

using namespace std::literals;

Sheet::~Sheet()
{
}

void Sheet::SetCell(Position pos, std::string text)
{
    if (!pos.IsValid())
        throw InvalidPositionException("Sheet::SetCell: Invalid position");
    if (!IsCellAvailable(pos))
        ExpandToPos(pos);

    table_.at(pos.row).at(pos.col)->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const
{
    if (!pos.IsValid())
        throw InvalidPositionException("Sheet::GetCell: Invalid position");

    if (IsCellAvailable(pos))
        return table_.at(pos.row).at(pos.col).get();
    else
        return nullptr;
}

CellInterface* Sheet::GetCell(Position pos)
{
    if (!pos.IsValid())
        throw InvalidPositionException("Sheet::GetCell: Invalid position");

    if (IsCellAvailable(pos))
        return table_.at(pos.row).at(pos.col).get();
    else
        return nullptr;
}

void Sheet::ClearCell(Position pos)
{
    if (!pos.IsValid())
        throw InvalidPositionException("Sheet::GetCell: Invalid position");

    if (IsCellAvailable(pos))
    {
        table_.at(pos.row).at(pos.col) = std::make_unique<Cell>(*this, pos);

        if (pos.row == size_area_.rows - 1)
        {
            bool empty = true;
            for (int i = pos.row; i >= 0 && empty; i--)
            {
                if (IsEmptyRow(i))
                    ResizeRow(i);
                else
                    empty = false;
            }
        }

        if (pos.col == size_area_.cols - 1)
        {
            bool empty = true;
            for (int i = pos.col; i >= 0 && empty; i--)
            {
                if (IsEmptyCol(i))
                    ResizeCol(i);
                else
                    empty = false;
            }
        }
    }
}

Size Sheet::GetPrintableSize() const
{
    return size_area_;
}

void Sheet::PrintValues(std::ostream& output) const
{
    auto caller = [&output](const auto& obj) { output << obj; };
    for (const auto& row : table_)
    {
        bool b = false;
        for (const auto& value : row)
        {
            if (b)
                output << '\t';
            if (value)
                std::visit(caller, value->GetValue());
            b = true;
        }
        output << std::endl;
    }
}

void Sheet::PrintTexts(std::ostream& output) const
{
    for (const auto& row : table_)
    {
        bool b = false;
        for (const auto& value : row)
        {
            if (b)
                output << '\t';
            if (value)
                output << value->GetText();
            b = true;
        }
        output << std::endl;
    }
}

bool Sheet::IsCellAvailable(Position pos) const
{
    return pos.IsValid() ? pos.row < size_area_.rows && pos.col < size_area_.cols : false;
}

void Sheet::ResizeCol(int new_size)
{
    if (new_size >= size_area_.cols) // Увеличивается
    {
        for (int i = 0; i < static_cast<int>(table_.size()); i++)
        {
            auto& row = table_.at(i);
            for (int j = size_area_.cols; j < new_size; j++)
                row.push_back(std::make_unique<Cell>(*this, Position{ i, j }));
        }
    }
    else
    {
        for (auto& row : table_)
            row.resize(new_size);
    }
    size_area_.cols = new_size;
}

void Sheet::ResizeRow(int new_size)
{
    table_.resize(new_size);  // Делаем ресайз строк в большую или меньшую сторону
    if (new_size >= size_area_.rows)
    {
        // Если ресайз в большую сторону, то нужно заполнить новые строки ячейками
        int size_col = !table_.empty() ? table_.at(0).size() : 0; // узнаём количество столбцов
        for (int i = size_area_.rows; i < static_cast<int>(table_.size()); i++)
        {
            // Начинаем проходить строки таблицы начиная со старого размера
            auto& row = table_.at(i);
            for (int j = 0; j < size_col; j++)
                row.push_back(std::make_unique<Cell>(*this, Position{ i, j }));
        }
    }
    size_area_.rows = new_size;
}

bool Sheet::IsEmptyCol(int col) const
{
    if (col >= size_area_.cols)
        return true;

    bool res = true;
    for (int i = 0; i < size_area_.rows && res; i++)
        res = table_.at(i).at(col)->Empty();

    return res;
}

bool Sheet::IsEmptyRow(int row) const
{
    if (row >= size_area_.rows)
        return true;

    bool res = true;
    for (int i = 0; i < size_area_.cols && res; i++)
        res = table_.at(row).at(i)->Empty();

    return res;
}

void Sheet::ExpandToPos(Position pos)
{
    if (pos.row >= size_area_.rows)
        ResizeRow(pos.row + 1);
    if (pos.col >= size_area_.cols)
        ResizeCol(pos.col + 1);
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}
