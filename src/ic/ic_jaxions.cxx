#include "ic_jaxions.h"
#include "../core/profiler.h"
#include "../io/io.h"


void JaxionsIC::apply(Field& field) const
{
    PROFILE(IC);

    const bool verb = field.verb();
    const std::string ic_file = p_->ic_file;

    IO reader(ic_file, FileMode::ReadOnly);
    reader.readJaxions(*p_, field);
    
    if (verb)
    {
        std::cout << "jaxions restart successful. Grid: N=" << field.size()
                  << ", dim=" << field.dim()
                  << ", sites=" << field.sites() << "\n";
    }
}
