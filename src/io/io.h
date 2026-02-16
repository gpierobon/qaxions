#ifndef IO_H
#define IO_H

#include <hdf5.h>
#include <string>
#include <vector>
#include <array>
#include "../parse.h"

enum class FileMode
{
    Create,       // create new file or truncate existing
    ReadOnly,     // open existing file read-only
    ReadWrite     // open existing file read-write 
};


class Field;  // Forward declaration

class IO
{
    public:
        IO(const std::string& filename, FileMode mode = FileMode::Create);
        ~IO();

        void flush();
        void close();

        void readConf(Params& p, Field& field); // For "native" grids
        void readJaxions(Params& p, Field& field); // For jaxions grids with m, v
        void writeConf(const Field& field, bool save_psi = false);

    private:
        hid_t file_id_;
        //bool append_mode_;

        template<typename T>
        T readAttribute(const std::string& group_name, 
                        const std::string& attr_name) const;

        template<typename T>
        void writeAttribute(const std::string& group_name,
                            const std::string& attr_name,
                            const T& value);

        void createGroup(const std::string& name);
        void readDataset(void* data, hid_t type_id, 
                         const std::string& dset_path);
        void writeDataset(const void* data, hid_t type_id,
                          const std::array<hsize_t,3>& dims,
                          const std::string& dset_name,
                          const std::string& group_name);
};

#endif
