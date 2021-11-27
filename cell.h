#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

class Sheet;


class Cell : public CellInterface
{
public:
    Cell(SheetInterface& sheet, Position pos);
    Cell(SheetInterface& sheet, Position pos, std::string text);
    virtual ~Cell() override = default;

    void                  Set(std::string text);
    void                  Clear();
    Value                 GetValue() const override;
    std::string           GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void                  SetPos(Position pos);
    bool                  IsReferenced() const;
    bool                  Empty() const;
    void                  ClearCache();

private:
    class Impl
    {
    public:
        enum class Type { EMPTY = 0, TEXT, FORMULA };
        virtual ~Impl() = default;

        virtual Value                 GetValue() const = 0;
        virtual std::string           GetText() const = 0;
        virtual Type                  GetType() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        static Type                   DefineType(const std::string& text);
    };

    class FormulaImpl : public Impl
    {
    public:
        FormulaImpl(std::string text, const SheetInterface& sheet);
        virtual ~FormulaImpl() override = default;

        virtual Type          GetType() const override;
        virtual Value         GetValue() const override;
        virtual std::string   GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        const SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> value_;
    };

    class EmptyImpl : public Impl
    {
    public:
        EmptyImpl() = default;
        virtual ~EmptyImpl() override = default;

        virtual Type          GetType() const override;
        virtual Value         GetValue() const override;
        virtual std::string   GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl
    {
    public:
        TextImpl(std::string text);
        virtual ~TextImpl() override = default;

        virtual Type          GetType() const override;
        virtual Value         GetValue() const override;
        virtual std::string   GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string value_;
    };

private:
    std::unique_ptr<Impl> MakeImpl(std::string text) const;   // Создание конкретной реализации значения ячейки
    bool CheckCyclicality(std::unique_ptr<Impl>& impl) const; // Проверка циклической зависимости
    bool IsCyclic(const Positions& dependents, Positions& viewed) const;

    void RemoveOldDependents();
    void AddNewDependents(const Positions& new_dependents);
    void AddReferencedCells(const std::vector<Position>& new_refs);
    void ResetCacheDependents();

private:
    SheetInterface& sheet_;               // Ссылка на таблицу, к которой принадлежит ячейка
    
    Position              pos_;           // Позиция ячейки
    std::unique_ptr<Impl> impl_;          // Значение ячейки таблицы

    Positions             dependents_;    // Зависимые ячейки (ячейки, на значения которых влияет данная ячейка)
    Positions             includes_;      // Используемые ячейки (ячейки, значения которых используются в данной ячейке)
    
    mutable std::optional<Value> cache_;  // Вычисленное значение
};
