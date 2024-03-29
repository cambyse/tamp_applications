#include <skeleton_printer.h>

#include <boost/algorithm/string/replace.hpp>

void PolicyPrinter::print( const Policy & policy )
{
  if( ! policy.root() )
  {
    return;
  }

  ss_ << "digraph g{" << std::endl;
  ss_ << "bgcolor=\"transparent\"";
  ss_ << "{" << std::endl;
  ss_ << policy.root()->id() << " [style=filled, fillcolor=blue]" << std::endl;

//  for( auto n : Policy.leaves() )
//  {
//    if( n )
//    {
//      ss_ << n->id() << " [style=filled, fillcolor=green]" << std::endl;
//    }
//  }
  ss_ << "}" << std::endl;

  saveGraphFrom( policy, policy.root() );

  ss_ << "}" << std::endl;
}

void PolicyPrinter::saveGraphFrom( const Policy & policy, const Policy::GraphNodeType::ptr & node )
{
  if( node->data().terminal )
  {
    ss_ << node->id() << " [style=filled, fillcolor=green]" << std::endl;
  }

  for( auto c : node->children() )
  {
    std::stringstream ss;
    std::string label;

    uint argIndex = 0;

    if( !c->data().leadingObservation.empty() )
    {
      ss << c->data().leadingObservation << "-" << std::endl;
    }

    for( auto arg : c->data().leadingKomoArgs )
    {
      ss << arg;

      if( argIndex == 0 )
      {
        ss << "(";
      }
      else if( argIndex < c->data().leadingKomoArgs.size() - 1 )
      {
        ss << ",";
      }

      argIndex++;
    }

    ss << ")";

    ss << std::endl;

    //
    //std::cout << "print return to " << c->data().decisionGraphNodeId << " = " << c->data().markovianReturn << std::endl;
    ss << "r=" << c->data().markovianReturn << std::endl;

    if( node->id() == 0 ) ss << "v=" << policy.value() << std::endl;

    label = ss.str();
    boost::replace_all(label, "{", "");

    ss_ << node->id() << "->" << c->id() << " [ label=\"" << label << "\" ]" << ";" << std::endl;

    saveGraphFrom( policy, c );
  }
}
