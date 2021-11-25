#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <set>

namespace
{
    class Formula : public FormulaInterface
    {
    public:
        explicit Formula(std::string expression)
            :ast_(ParseFormulaAST(expression))
        {
        }

        Value Evaluate(const SheetInterface& sheet) const override
        {
            CellValue cell_value = [&sheet](Position pos)
            {
                if (!sheet.GetCell(pos))
                    return 0.0;
                auto value = sheet.GetCell(pos)->GetValue();
                if (std::holds_alternative<double>(value))
                {
                    return std::get<double>(value);
                }
                else if (std::holds_alternative<std::string>(value))
                {
                    std::string text = sheet.GetCell(pos)->GetText();

                    if (text.empty())
                        return 0.0;
                    if (text.front() == ESCAPE_SIGN)
                        throw FormulaError(FormulaError::Category::Value);

                    try
                    {
                        return std::stod(text);
                    }
                    catch (...)
                    {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                else
                {
                    throw std::get<FormulaError>(value);
                }
            };

            try
            {
                return ast_.Execute(cell_value);
            }
            catch (const FormulaError& fe)
            {
                return fe;
            }

        }

        std::string GetExpression() const override
        {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override
        {
            auto positions = ast_.GetCells();
            std::set<Position> unique_ref = { positions.begin(), positions.end() };
            return { unique_ref.begin(), unique_ref.end() };
        }

        virtual ~Formula() override = default;

    private:
        FormulaAST ast_;
    };

}  // namespace


std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    return std::make_unique<Formula>(std::move(expression));
}