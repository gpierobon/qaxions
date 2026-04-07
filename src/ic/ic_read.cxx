#include "ic_read.h"
#include "../core/profiler.h"
#include "../io/io.h"


void ReadConfIC::apply(Field& field) const
{
    PROFILE(IC);

    const bool verb = field.verb();
    //const std::string ic_file = p_->ic_file;
    const std::string ic_file = (std::filesystem::path(p_->dir) / p_->ic_file).string();
    std::cout << ic_file << std::endl;

    IO reader(ic_file, FileMode::ReadOnly);
    reader.readConf(*p_, field);
    
    if (verb)
    {
        std::cout << "ReadConf restart successful. Grid: N=" << field.size()
                  << ", dim=" << field.dim()
                  << ", sites=" << field.sites() << "\n";
    }
}
