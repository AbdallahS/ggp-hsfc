/*****************************************************************************************
 * This module provides converters between text formatted as S-expressions and the flat
 * string format used by the internals of the HSFC. In the latter case the  arity of 
 * predicate/functions is supplied to provide the appropriate associations of terms.
 * Each term is given an arity (although an arity of 0 can be dropped).
 *
 * For example: 
 *                S-expression                       flat format
 *                ------------                       -----------
 *                 (single)             <=>           single       (note: dropped 0 arity)     
 * (this (is an) (example (of a text))) <=> this|2 is|1 an|0 example|1 of|2 a|0 text|0
 *
 * Notes/limitations: 
 *
 * 1) in its current form the flat format is not quite as general as S-expression since 
 *    it assumes that the first element is a simple term. For example, "((a b) c)" cannot 
 *    be represented in the flat format. To do so would need to allow for "a|1|1 b c".
 *
 * 2) Not going to handle character escaping. So we disallow "|" in S-expressions, and
 *    "(" and ")" in the flat format.
 *
 *****************************************************************************************/
#ifndef HSFC_SEXPRTOFLAT_H
#define HSFC_SEXPRTOFLAT_H

#include <string>
#include <vector>
#include <boost/variant.hpp>

namespace HSFC
{

/*****************************************************************
 * Term is defined recursively
 *****************************************************************/

struct Term;

typedef boost::variant<
	std::string,
	boost::recursive_wrapper<Term>
	> TermNode;
					   
struct Term
{
	std::vector<TermNode> children_;
};

/*****************************************************************
 * Will throw HSFCException if the strings include disallow chars.
 *****************************************************************/

std::string sexpr_to_flat(const std::string& sexpr);
std::string flat_to_sexpr(const std::string& flat);

void parse_sexpr(const std::string& sexpr, Term& term);
void parse_flat(const std::string& flat, Term& term);

std::ostream& generate_sexpr(const Term& term, std::ostream& os);
std::ostream& generate_flat(const Term& term, std::ostream& os);

};

#endif /* HSFC_SEXPRTOFLAT_H */
