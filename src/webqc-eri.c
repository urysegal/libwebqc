#include "webqc-handler.h"
#include "libwebqc.h"


static void
print_system_sizes(const struct ERI_information *eri, FILE *fp)
{
    fprintf(fp,"%u atoms, %u electrons, %u shells, %u functions, %u ERI integrals (%u primitives)\n",
    eri-> number_of_atoms,
    eri-> number_of_electrons,
    eri-> number_of_shells,
    eri-> number_of_functions,
    eri-> number_of_integrals,
    eri-> number_of_primitives);
}

static void print_primitives(const struct ERI_information *eri,  FILE *fp)
{
    fprintf(fp, "id\tcoefficient\texponent\n");

    for (int i = 0U; i < eri->number_of_primitives; ++i) {
        const struct radial_function_info *func = &eri->basis_function_primitives[i];
        fprintf(fp, "%u\t%f\t%f\n", i, func->coefficient, func->exponent);
    }
}

static void print_basis_set(const struct ERI_information *eri,  FILE *fp)
{
    fprintf(fp,"AtomID\tElem\tShell\tam\tCrtsian\tFunc\tPrims\tFirst\n");
    for ( int i = 0U ; i < eri->number_of_functions ; ++i ) {
        const struct basis_function_instance *func = &eri->basis_functions[i];
        fprintf(fp, "%u\t%s\t%u\t%u\t%s\t%s\t%u\t%u\n",
                func->atom_index,
                func->element_symbol,
                func->shell_index,
                func->angular_moment_l,
                func->coordinate_type == WQC_CARTESIAN ? "Yes" : "No",
                func->function_label,
                func->number_of_primitives,
                func->first_primitives);
    }
}


void wqc_print_integrals_details( WQC *handler, FILE *fp)
{
    struct ERI_information *eri = &handler->eri_info;

    print_system_sizes(eri, fp);

    print_basis_set(eri, fp);

    print_primitives(eri, fp);

}
