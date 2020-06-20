install(FILES
    "2014-11 Performance and Benchmarking.docx"
    "2014-11 Performance and Benchmarking.pdf"
    "Alg-dds_x.pdf"
    "Alg-dds_x.rtf"
    "DLL-dds_x.htm"
    "DLL-dds_x.mht"
    "DLL-dds_x.pdf"
    "DLL-dds_x.rtf"
    "dll-description.html"
    "dll-description.md"
    "dll-description.pdf"
    "Par Calculation.pdf"
    "Par Calculation.rtf"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    COMPONENT Docs)

install(FILES
    "DLL-dds_x-Dateien/colorschememapping.xml"
    "DLL-dds_x-Dateien/filelist.xml"
    "DLL-dds_x-Dateien/header.htm"
    "DLL-dds_x-Dateien/item0001.xml"
    "DLL-dds_x-Dateien/props002.xml"
    "DLL-dds_x-Dateien/themedata.thmx"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}/DLL-dds_x-Dateien"
    COMPONENT Docs)
