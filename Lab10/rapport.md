### Rapport Final Labo 10 Harun Ouweis

---

#### a) Allocation Dynamique dans SO3
Le noyau SO3 gère son tas via `so3/mm/heap.c`, utilisant une allocation dynamique pour optimiser l'utilisation de la mémoire, heap.c se trouve ici dansle répertoire mm (gestion mémoire) car il est intrinsèquement lié à la gestion de la mémoire dans le noyau. mm signifie "memory management", nous avons dans mm aussi d'autres fichiers liés notamment à la pagination, et à d'autres aspects de la gestion de la mémoire virtuelle et physique qui nous indique qu'on est bien dans le bon fichier. (Page dans `so3/mm/memory.c`).

#### b) Stratégie de Gestion Mémoire
`heap.c` emploie Quick Fit, une stratégie efficace réduisant la fragmentation en fusionnant des blocs adjacents libres et en triant la mémoire disponible pour des allocations rapides.

#### c) Gestion de la MMU
Les fonctions dans `mmu.c` sont essentielles pour la gestion des tables de pages, assurant des mappages précis entre les adresses virtuelles et physiques, et gérant les droits d'accès à la mémoire.

#### f) Observations sur l'Application `memory`
Les adresses virtuelles restent constantes à chaque exécution, indiquant une zone d'adressage virtuel stable. Les adresses physiques varient, reflétant le dynamisme de l'allocation de la mémoire physique et l'indépendance du mappage virtuel-physique.

##### Résultat obtenus

```
so3% memory
Adresse virtuelle du buffer: 0x20d018
Adresse physique correspondante: 48bc0018
so3% 
so3% memory
Adresse virtuelle du buffer: 0x20d018
Adresse physique correspondante: 4cfda018
so3% memory
Adresse virtuelle du buffer: 0x20d018
Adresse physique correspondante: 513f4018
so3% memory
Adresse virtuelle du buffer: 0x20d018
Adresse physique correspondante: 5580e018
so3% memory
Adresse virtuelle du buffer: 0x20d018
Adresse physique correspondante: 59c28018
```

```
so3% memory
Adresse virtuelle du buffer: 0x20d018
Adresse physique correspondante: [différente à chaque exécution]
```

### Explications 

En exécutant memory, on constate que les adresses virtuelles ne changent pas, ce qui montre que le système réserve le même emplacement dans l'espace virtuel à chaque fois. Cela illustre bien le principe de la pagination, où le système crée un lien stable entre la mémoire virtuelle que voit le programme et la mémoire physique réelle. Les adresses physiques, elles, varient à chaque lancement, indiquant que le système choisit différents emplacements physiques pour stocker les données, une manière de gérer la mémoire qui est flexible et sécurisée.
---

