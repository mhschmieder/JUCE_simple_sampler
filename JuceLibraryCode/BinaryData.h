/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_22039042_INCLUDED
#define BINARYDATA_H_22039042_INCLUDED

namespace BinaryData
{
    extern const char*   Tranche1_aif;
    const int            Tranche1_aifSize = 264660;

    extern const char*   Tranche2_aif;
    const int            Tranche2_aifSize = 264660;

    extern const char*   Tranche3_aif;
    const int            Tranche3_aifSize = 264660;

    extern const char*   Tranche4_aif;
    const int            Tranche4_aifSize = 264660;

    extern const char*   Tranche5_aif;
    const int            Tranche5_aifSize = 264660;

    extern const char*   Tranche6_aif;
    const int            Tranche6_aifSize = 264660;

    extern const char*   Tranche7_aif;
    const int            Tranche7_aifSize = 264660;

    extern const char*   Tranche8_aif;
    const int            Tranche8_aifSize = 264654;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 8;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
