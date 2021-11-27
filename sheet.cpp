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

    if (!text.empty())
        positions_.insert(pos);

    if (!table_.count(pos))
        table_[pos] = std::make_unique<Cell>(*this, pos, std::move(text));
    else
        table_[pos]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const
{
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos)
{
    if (!pos.IsValid())
        throw InvalidPositionException("Sheet::GetCell: Invalid position");

    if (IsCellAvailable(pos))
    {
        if (!table_.count(pos))
            SetCell(pos, std::string());

        return table_.at(pos).get();
    }
    else
    {
        return nullptr;
    }


}

void Sheet::ClearCell(Position pos)
{
    if (IsCellAvailable(pos))
        SetCell(pos, std::string());
    else
        table_.erase(pos);
    positions_.erase(pos);
}

Size Sheet::GetPrintableSize() const
{
    int max_r = 0;
    int max_c = 0;

    for (Position pos : positions_)
    {
        max_r = std::max(max_r, pos.row + 1);
        max_c = std::max(max_c, pos.col + 1);
    }

    return { max_r, max_c };
}

void Sheet::PrintValues(std::ostream& output) const
{
    auto caller = [&output](const auto& obj) { output << obj; };
    Size scope = GetPrintableSize();

    for (int i = 0; i < scope.rows; ++i)
    {
        bool b = false;
        for (int j = 0; j < scope.cols; ++j)
        {
            Position pos{ i, j };
            auto cell = GetCell(pos);

            if (b)
                output << '\t';
            if (cell)
                std::visit(caller, cell->GetValue());
            b = true;
        }
        output << std::endl;
    }
}

void Sheet::PrintTexts(std::ostream& output) const
{
    auto caller = [&output](const auto& obj) { output << obj; };
    Size scope = GetPrintableSize();

    for (int i = 0; i < scope.rows; ++i)
    {
        bool b = false;
        for (int j = 0; j < scope.cols; ++j)
        {
            Position pos{ i, j };
            auto cell = GetCell(pos);

            if (b)
                output << '\t';
            if (cell)
                output << cell->GetText();
            b = true;
        }
        output << std::endl;
    }
}


std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}
