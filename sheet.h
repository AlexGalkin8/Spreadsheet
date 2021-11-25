#pragma once

#include "common.h"
#include "cell.h"

#include <functional>
#include <vector>

using Table = std::vector<std::vector<std::unique_ptr<Cell>>>;

class Sheet : public SheetInterface
{
public:
    Sheet() = default;
    virtual ~Sheet() override;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;

    void PrintTexts(std::ostream& output) const override;

private:
    // Методы редактирования
    void ResizeCol(int new_size);             // Изменяет размер указанного столбца
    void ResizeRow(int new_size);             // Изменяет размер до указанной строки
    void ExpandToPos(Position pos);           // Расширяет область согласно указанной позиции

    bool IsCellAvailable(Position pos) const; // Находится ли переданная позиция в пределах зоны таблицы
    bool IsEmptyCol(int col) const;           // Пустой ли указанный столбец
    bool IsEmptyRow(int row) const;           // Пустая ли указанная строка

private:
    Table table_;
    Size size_area_;
};