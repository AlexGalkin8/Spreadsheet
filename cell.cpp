#include "cell.h"


/********************   Cell   ********************/

Cell::Cell(SheetInterface& sheet, Position pos)
    : sheet_(sheet)
    , pos_(pos)
    , impl_(std::make_unique<EmptyImpl>())
    , dependents_()
    , includes_()
    , cache_(std::nullopt)
{
}

void Cell::Set(std::string text)
{
    auto value = MakeImpl(std::move(text)); // Задаём значение ячейки
    if (CheckCyclicality(value))
        throw CircularDependencyException("Cyclic dependency detected");

    ClearCache();
    RemoveOldDependents();
    AddReferencedCells(value->GetReferencedCells());
    AddNewDependents(includes_);
    ResetCacheDependents();

    impl_ = std::move(value);
}

void Cell::Clear()
{
    impl_ = std::make_unique<EmptyImpl>();
}

void Cell::ClearCache()
{
    cache_.reset();
}

Cell::Value Cell::GetValue() const
{
    if (!cache_.has_value())
        cache_ = impl_->GetValue();
    return cache_.value();
}

std::string Cell::GetText() const
{
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

void Cell::SetPos(Position pos)
{
    pos_ = pos;
}

inline bool Cell::IsReferenced() const
{
    return !dependents_.empty() || !includes_.empty();
}

bool Cell::Empty() const
{
    return impl_->GetType() == Impl::Type::EMPTY;
}

std::unique_ptr<Cell::Impl> Cell::MakeImpl(std::string text) const
{
    Impl::Type value_type = Impl::DefineType(text);

    if (value_type == Impl::Type::FORMULA)
        return std::make_unique<FormulaImpl>(std::move(text), sheet_);
    else if (value_type == Impl::Type::EMPTY)
        return std::make_unique<EmptyImpl>();
    else
        return std::make_unique<TextImpl>(std::move(text));
}

bool Cell::CheckCyclicality(std::unique_ptr<Cell::Impl>& impl) const
{
    auto positions = impl->GetReferencedCells();
    Positions dependents(positions.begin(), positions.end());
    Positions checkeds;
    return IsCyclic(dependents, checkeds);
}

bool Cell::IsCyclic(const Positions& dependents, Positions& verified) const
{
    if (dependents.count(pos_))
        return true;
    for (Position pos : dependents)
    {
        if (pos.IsValid() && !verified.count(pos))
        {
            verified.insert(pos);
            auto cell = sheet_.GetCell(pos);
            if (cell)
            {
                auto ref = cell->GetReferencedCells();
                if (IsCyclic({ ref.begin(), ref.end() }, verified))
                    return true;
            }
        }
    }
    return false;
}

void Cell::RemoveOldDependents()
{
    for (Position pos : dependents_)
    {
        Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (!pos.IsValid() || !cell)
            continue;
        cell->includes_.erase(pos);
    }
}

void Cell::AddNewDependents(const Positions& new_dependents)
{
    for (Position pos : new_dependents)
    {
        Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (!pos.IsValid() || !cell)
            continue;

        cell->AddDependents(pos);
    }
}

void Cell::RemoveDependents(Position pos)
{
    dependents_.erase(pos);
}

void Cell::AddDependents(Position pos)
{
    dependents_.insert(pos);
}

void Cell::ResetCacheDependents()
{
    for (Position pos : includes_)
    {
        Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        if (!pos.IsValid() || !cell)
            continue;
        cell->ClearCache();
        cell->ResetCacheDependents();
    }
}

void Cell::AddReferencedCells(const std::vector<Position>& new_refs)
{
    includes_.clear();
    if (!new_refs.empty())
        includes_.insert(new_refs.begin(), new_refs.end());

}



/********************   Cell::Impl   ********************/

Cell::Impl::Type Cell::Impl::DefineType(const std::string& text)
{
    if (text.empty())
        return Type::EMPTY;
    else if (text.at(0) == FORMULA_SIGN && text.size() > 1)
        return Type::FORMULA;
    else
        return Type::TEXT;
}


/********************   Cell::EmptyImpl   ********************/

Cell::EmptyImpl::Type Cell::EmptyImpl::GetType() const
{
    return Type::EMPTY;
}

Cell::Value Cell::EmptyImpl::GetValue() const
{
    return std::string();
}

std::string Cell::EmptyImpl::GetText() const
{
    return std::string();
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const
{
    return {};
}


/********************   Cell::TextImpl   ********************/

Cell::TextImpl::TextImpl(std::string text)
    :value_(std::move(text))
{
}

Cell::TextImpl::Type Cell::TextImpl::GetType() const
{
    return Type::TEXT;
}

Cell::Value Cell::TextImpl::GetValue() const
{
    if (!value_.empty() && value_.at(0) == ESCAPE_SIGN)
        return value_.substr(1);
    else
        return value_;
}

std::string Cell::TextImpl::GetText() const
{
    return value_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const
{
    return {};
}


/********************   Cell::FormulaImpl   ********************/

Cell::FormulaImpl::FormulaImpl(std::string text, const SheetInterface& sheet)
    : sheet_(sheet)
    , value_(ParseFormula(text.substr(1))) // Обрезаем '='

{
}

Cell::FormulaImpl::Type Cell::FormulaImpl::GetType() const
{
    return Type::FORMULA;
}

Cell::Value Cell::FormulaImpl::GetValue() const
{
    auto out_value = value_->Evaluate(sheet_);
    if (std::holds_alternative<double>(out_value))
        return std::get<double>(out_value);
    else
        return std::get<FormulaError>(out_value);
}

std::string Cell::FormulaImpl::GetText() const
{
    return FORMULA_SIGN + value_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return value_->GetReferencedCells();
}