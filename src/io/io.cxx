#include <H5Cpp.h>
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "io.h"
#include "../field.h"
#include "../profiler.h"


IO::IO(const std::string& filename, FileMode mode)
{
    switch (mode)
    {
        case FileMode::Create:
            file_id_ = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC,
                                 H5P_DEFAULT, H5P_DEFAULT);
            break;

        case FileMode::ReadOnly:
            file_id_ = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
            break;

        case FileMode::ReadWrite:
            file_id_ = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
            break;
    }

    if (file_id_ < 0)
    {
        std::string msg = "Failed to ";
        if (mode == FileMode::Create) msg += "create/truncate";
        else if (mode == FileMode::ReadOnly) msg += "open (read-only)";
        else msg += "open (read-write)";
        msg += " HDF5 file: " + filename;
        throw std::runtime_error(msg);
    }
    // store mode if needed later?
    // mode_ = mode;
}


void IO::flush()
{
    if (file_id_ >= 0) {
        H5Fflush(file_id_, H5F_SCOPE_GLOBAL);
    }
}


void IO::close()
{
    if (file_id_ >= 0) {
        H5Fflush(file_id_, H5F_SCOPE_GLOBAL);
        H5Fclose(file_id_);
        file_id_ = -1;
    }
}


IO::~IO()
{
    close();
}


template<typename T>
T IO::readAttribute(const std::string& group_name, const std::string& attr_name) const
{
    if (file_id_ < 0) 
        throw std::runtime_error("File not open");

    hid_t gid = H5Gopen2(file_id_, group_name.c_str(), H5P_DEFAULT);
    if (gid < 0) 
        throw std::runtime_error("Cannot open group: " + group_name);

    hid_t attr = H5Aopen(gid, attr_name.c_str(), H5P_DEFAULT);
    if (attr < 0) {
        H5Gclose(gid);
        throw std::runtime_error("Attribute '" + attr_name + "' not found in group: " + group_name);
    }

    T value;
    hid_t type_id = H5T_NATIVE_INT;     // default
    if constexpr (std::is_same_v<T, double>) {
        type_id = H5T_NATIVE_DOUBLE;
    } else if constexpr (std::is_same_v<T, int>) {
        type_id = H5T_NATIVE_INT;
    } else if constexpr (std::is_same_v<T, float>) {
        type_id = H5T_NATIVE_FLOAT;
    } // might need to add for string

    herr_t status = H5Aread(attr, type_id, &value);
    H5Aclose(attr);
    H5Gclose(gid);

    if (status < 0) 
        throw std::runtime_error("Failed to read attribute '" + attr_name + "' in group: " + group_name);
    return value;
}


template<typename T>
void IO::writeAttribute(const std::string& group_name,
                        const std::string& attr_name,
                        const T& value)
{
    if (file_id_ < 0) 
        throw std::runtime_error("File not open");

    hid_t gid = H5Gopen2(file_id_, group_name.c_str(), H5P_DEFAULT);
    if (gid < 0)
    {
        gid = H5Gcreate2(file_id_, group_name.c_str(),
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (gid < 0) 
            throw std::runtime_error("Failed to create group: " + group_name);
    }

    hsize_t adim = 1;
    hid_t aspace = H5Screate_simple(1, &adim, nullptr);
    if (aspace < 0)
    {
        H5Gclose(gid);
        throw std::runtime_error("Failed to create attribute dataspace");
    }

    hid_t type_id = H5T_NATIVE_DOUBLE;  // default
    if constexpr (std::is_same_v<T, int>) 
        type_id = H5T_NATIVE_INT;
    else if constexpr (std::is_same_v<T, double>)
        type_id = H5T_NATIVE_DOUBLE;
    else if constexpr (std::is_same_v<T, float>)
        type_id = H5T_NATIVE_FLOAT;
    else if constexpr (std::is_same_v<T, bool>)
        type_id = H5T_NATIVE_HBOOL;
    else if constexpr (std::is_same_v<T, unsigned int>)
        type_id = H5T_NATIVE_UINT;
    // Missing anything?

    hid_t attr = H5Acreate2(gid, attr_name.c_str(),
                            type_id, aspace,
                            H5P_DEFAULT, H5P_DEFAULT);
    if (attr < 0)
    {
        H5Sclose(aspace);
        H5Gclose(gid);
        throw std::runtime_error("Failed to create attribute: " + attr_name);
    }

    herr_t status = H5Awrite(attr, type_id, &value);
    if (status < 0)
    {
        H5Aclose(attr);
        H5Sclose(aspace);
        H5Gclose(gid);
        throw std::runtime_error("Failed to write attribute: " + attr_name);
    }

    H5Aclose(attr);
    H5Sclose(aspace);
    H5Gclose(gid);
}


void IO::createGroup(const std::string& name)
{
    herr_t status = H5Lexists(file_id_, name.c_str(), H5P_DEFAULT);
    if (status > 0) {
        return;
    }
    if (status < 0) {
        throw std::runtime_error("H5Lexists failed for group: " + name);
    }

    hid_t gid = H5Gcreate2(file_id_, name.c_str(),
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (gid < 0) {
        throw std::runtime_error("Failed to create group: " + name);
    }
    H5Gclose(gid);
}


void IO::readDataset(void* data_out, hid_t type_id, const std::string& dset_path)
{
    hid_t dset = H5Dopen2(file_id_, dset_path.c_str(), H5P_DEFAULT);

    if (dset < 0)
        throw std::runtime_error("Cannot open dataset: " + dset_path);

    // Get dataspace
    hid_t filespace = H5Dget_space(dset);

    // Get dataset
    herr_t status = H5Dread(dset, type_id, H5S_ALL, filespace, H5P_DEFAULT, data_out);

    H5Sclose(filespace);
    H5Dclose(dset);

    if (status < 0)
        throw std::runtime_error("Failed to read dataset: " + dset_path);
}


void IO::writeDataset(const void* data, hid_t type_id,
                      const std::array<hsize_t,3>& dims,
                      const std::string& dset_name,
                      const std::string& group_name)
{
    std::string full_path = group_name + "/" + dset_name;

    hid_t filespace = H5Screate_simple(3, dims.data(), nullptr);
    if (filespace < 0) 
        throw std::runtime_error("Failed to create dataspace for: " + full_path);
    
    hid_t dset = H5Dcreate2(file_id_, full_path.c_str(),
                            type_id, filespace,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dset < 0) {
        H5Sclose(filespace);
        throw std::runtime_error("Failed to create dataset: " + full_path);
    }

    herr_t status = H5Dwrite(dset, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    H5Dclose(dset);
    H5Sclose(filespace);

    if (status < 0)
        throw std::runtime_error("Failed to write dataset: " + full_path);
}


void IO::readJaxions(Params& pars, Field& field)
{
    PROFILE(IO)

    if (file_id_ < 0)
        throw std::runtime_error("HDF5 file not open for reading");

    const std::string header = "Header";

    int N       = readAttribute<int>(header, "N");
    int dim     = readAttribute<int>(header, "dim");
    double Lbox = readAttribute<double>(header, "L");

    pars.N = N;
    pars.Lbox = Lbox;
    pars.dim = dim;

    field.init(pars);

    // 3. Read m, v arrays
    std::vector<double> real(field.sites());
    readDataset(real.data(), H5T_NATIVE_DOUBLE, "/m");

    std::vector<double> imag(field.sites());
    readDataset(imag.data(), H5T_NATIVE_DOUBLE, "/v");

    fftw_complex* psi = field.psi();

    #pragma omp parallel for
    for (size_t i = 0; i < field.sites(); ++i)
    {
        psi[i][0] = real[i]; // To add the normalisation
        psi[i][1] = imag[i];
    }
}


void IO::readConf(Params& pars, Field& field)
{
    PROFILE(IO)

    if (file_id_ < 0)
        throw std::runtime_error("HDF5 file not open for reading");
    
    const std::string header = "Header";

    int N       = readAttribute<int>(header, "N");
    int dim     = readAttribute<int>(header, "dim");
    double Lbox = readAttribute<double>(header, "Lbox");

    pars.N = N;
    pars.Lbox = Lbox;
    pars.dim = dim;

    field.init(pars);
    
    // 3. Read m, v arrays
    std::vector<double> real(field.sites());
    readDataset(real.data(), H5T_NATIVE_DOUBLE, "/psi/real");

    std::vector<double> imag(field.sites());
    readDataset(imag.data(), H5T_NATIVE_DOUBLE, "/psi/imag");

    fftw_complex* psi = field.psi();

    #pragma omp parallel for
    for (size_t i = 0; i < field.sites(); ++i)
    {
        psi[i][0] = real[i];
        psi[i][1] = imag[i];
    }

}


void IO::writeConf(const Field& field, bool save_psi)
{
    PROFILE(IO)
    const int N = field.size();
    const int dim = field.dim();
    const int curr = field.curr();
    const double time = field.time();
    const size_t sites = field.sites();

    std::array<hsize_t,3> dims;
    if (dim == 3) 
        dims = {static_cast<hsize_t>(N), static_cast<hsize_t>(N), static_cast<hsize_t>(N)};
    else // dim == 2
        dims = {static_cast<hsize_t>(N), static_cast<hsize_t>(N), 1};
    
    std::vector<double> rho(sites);
    const double* V = field.V();

    #pragma omp parallel for
    for (size_t i = 0; i < sites; ++i)
        rho[i] = V[i];
    
    const std::string rho_group = "rho";
    createGroup(rho_group);
    writeDataset(rho.data(), H5T_NATIVE_DOUBLE, dims, "data", rho_group);

    if (save_psi)
    {
        std::vector<double> real_part(sites), imag_part(sites);
        const fftw_complex* psi = field.psi();
        
        #pragma omp parallel for
        for (size_t i = 0; i < sites; ++i)
        {
            real_part[i] = psi[i][0]; 
            imag_part[i] = psi[i][1];
        }

        const std::string psi_group = "psi";
        createGroup(psi_group);
        writeDataset(real_part.data(), H5T_NATIVE_DOUBLE, dims, "real", psi_group);
        writeDataset(imag_part.data(), H5T_NATIVE_DOUBLE, dims, "imag", psi_group);
    }
    
    
    createGroup("Header");
    writeAttribute<int>   ("Header", "N",     N);
    writeAttribute<int>   ("Header", "dim",   dim);
    writeAttribute<int>   ("Header", "step",  curr);
    writeAttribute<double>("Header", "time",  time);
    writeAttribute<double>("Header", "Lbox",  field.Lbox());

}

