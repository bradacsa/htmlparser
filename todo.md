2022.09.27.

- [x] A nem lezárt selfclosing tageket is ki kell szűrni, amit az emittingben kell majd megnézni
    - [x] Előellenőrzés az OPEN_TAG és CLOSE_TAG-ek számára, stackkeléssel tagname alapján
    - [x] Aminek nincs END_TAG párja, az SELFCLOSING_TAG-gé válik
    - [] Tagname alapjáni szűrés?
- [] Searchbys bővítése és kidolgozása -> searchBy() funkció
    - [] Több féle paraméter alapján is lehessen keresni egyszerre