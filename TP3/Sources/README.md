
# TP3

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`

## lscpu

Architecture:                    x86_64
CPU op-mode(s):                  32-bit, 64-bit
Byte Order:                      Little Endian
Address sizes:                   39 bits physical, 48 bits virtual
CPU(s):                          4
On-line CPU(s) list:             0-3
Thread(s) per core:              2
Core(s) per socket:              2
Socket(s):                       1
NUMA node(s):                    1
Vendor ID:                       GenuineIntel
CPU family:                      6
Model:                           142
Model name:                      Intel(R) Core(TM) i7-7500U CPU @ 2.70GHz
Stepping:                        9
CPU MHz:                         806.474
CPU max MHz:                     3500.0000
CPU min MHz:                     400.0000
BogoMIPS:                        5799.77
Virtualization:                  VT-x
L1d cache:                       64 KiB
L1i cache:                       64 KiB
L2 cache:                        512 KiB
L3 cache:                        4 MiB
NUMA node0 CPU(s):               0-3
Vulnerability Itlb multihit:     KVM: Mitigation: Split huge pages
Vulnerability L1tf:              Mitigation; PTE Inversion; VMX conditional cache flushes, SMT vulnerable
Vulnerability Mds:               Mitigation; Clear CPU buffers; SMT vulnerable
Vulnerability Meltdown:          Mitigation; PTI
Vulnerability Spec store bypass: Mitigation; Speculative Store Bypass disabled via prctl and seccomp
Vulnerability Spectre v1:        Mitigation; usercopy/swapgs barriers and __user pointer sanitization
Vulnerability Spectre v2:        Mitigation; Full generic retpoline, IBPB conditional, IBRS_FW, STIBP conditional, RSB filling
Vulnerability Srbds:             Mitigation; Microcode
Vulnerability Tsx async abort:   Not affected
Flags:                           fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse
                                 2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopolog
                                 y nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg fma cx16 xtpr p
                                 dcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnow
                                 prefetch cpuid_fault epb invpcid_single pti ssbd ibrs ibpb stibp tpr_shadow vnmi flexpriority ept vpid ep
                                 t_ad fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid mpx rdseed adx smap clflushopt intel_pt xsaveop
                                 t xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp md_clear flush_l1d


## Produit scalaire 

OMP_NUM    | samples=1024 | 
-----------|--------------|----------
séquentiel |  S = 1       | 
1          |  S = 1       | 
2          |  S = 1.25    | 
3          |  S = 1.28    | 
4          |  S = 1.02    | 
8          |  S = 0.94    | 

On constate qu'il y a un memory bound : le temps de calcul est très faible, ce qui est long, c'est l'écriture, car on ecrit toujours au même endroit
Vu que ma machine a 4 Threads (2 coeurs), c'est normal que ce soit décéléré pour 8 threads. 

threads    | samples=1024 | 
-----------|--------------|----------
séquentiel |  S = 1       | 
1          |  S = 1       | 
2          |  S = 1.18    | 
3          |  S = 1.11    | 
4          |  S = 1.05    | 
8          |  S = 0.86    | 

En comparant ces résultats avec ceux d'Open MP, on remarque qu'il y a une certaine cohérence du fait que 
dans les deux cas S > 1 tant que 1 < num_thread < 5. Les résults d'Open MP sont par contre meilleurs du faits 
qu'ils offrent une meilleure accélération.

On peut pas améliorer à cause des taches non parallélisables.
## Produit matrice-matrice
Pour 1023 : 6.64049 secondes
Pour 1024 : 25.4070 secondes
Pour 1023 : 7.23941 secondes

Le temps pour une taille de 1024 est significativement plus long que pour 1023 car le processeur atteint la limite de sa mémoire cache, il est donc obligé de mettre des données en RAM

### Permutation des boucles

*Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :*

`make TestProduct.exe && ./TestProduct.exe 1024`


  ordre           | time    | MFlops  | MFlops(n=2048) 
------------------|---------|---------|----------------
i,j,k (origine)   | 151     | 113     |                
j,i,k             | 128.207 | 134.001 |    
i,k,j             | 365.278 | 47.0323 |    
k,i,j             | 195.586 | 87.8379 |    
j,k,i             | 43.6695 | 393.407 |    
k,j,i             | 39.5948 | 433.829 |    


*Discussion des résultats*
Le pronleme avec les boucles autres que (k,j,i) et (j, k, i), c'est que l'on accède trop souvent à la mémoire, on est donc dans un cas memory bound, autrement dit, on est limité par la capacité d'accès a la mémoire. 



### OMP sur la meilleure boucle 

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

  OMP_NUM         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
1                 |         |    445.072     |
2                 |         |    433.668
3                 |         |    432.858
4                 |         |    415.194
5                 |         |    436.819
6                 |         |    487.762
7                 |         |    481.806
8                 |         |    493.446




### Produit par blocs

`make TestProduct.exe && ./TestProduct.exe 1024`

  szBlock         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
origine (=max)    |  |
32                |  |
64                |  |
128               |  |
256               |  |
512               |  | 
1024              |  |




### Bloc + OMP



  szBlock      | OMP_NUM | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
---------------|---------|---------|------------------------------------------------
A.nbCols       |  1      |         | 
512            |  8      |         | 



## Bouddha 
Sans parallélisation, le temps de calcul sont de 6.2, 5.7 et 0.5 respectivement pour Bouddha1, Bouddha2 et Bouddha3. Avec parallelisation, ces temps passent à 13.5, 12.8 et 1.2 mais les taches s'effectuent en meme temps. Il n'y a donc pas d'accéleration. 



# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
