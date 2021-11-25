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
    // ������ ��������������
    void ResizeCol(int new_size);             // �������� ������ ���������� �������
    void ResizeRow(int new_size);             // �������� ������ �� ��������� ������
    void ExpandToPos(Position pos);           // ��������� ������� �������� ��������� �������

    bool IsCellAvailable(Position pos) const; // ��������� �� ���������� ������� � �������� ���� �������
    bool IsEmptyCol(int col) const;           // ������ �� ��������� �������
    bool IsEmptyRow(int row) const;           // ������ �� ��������� ������

private:
    Table table_;
    Size size_area_;
};