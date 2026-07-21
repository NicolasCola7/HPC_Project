# Questo script di gnuplot può essere utilizzato per mostrare il
# risultato dell'autocorrelazione. Dopo aver creato il file di ouptut
# di nome "out1M", eseguire questo script con il comando:
#
# gnuplot plot.gp
#
# per ottenere una immagine "out1M.png"

# Commentato per produrre direttamente una immagine in formato
# .png senza passare dall'Encapsulated Postsccript.
# set term postscript eps color size 6,4
# set output "out1M.eps"
set term png size 1920,1280 lw 2
set output "out1M.png"
set multiplot layout 3,1
set lmargin at screen .1

set title "Original data"
set ylabel "X_i"
plot [1:2048] \
     "data1M_clean.txt" with l lw 2 notitle

set title "Noisy data"
set ylabel "X_i + rand"
plot [1:2048] \
     "data1M_noisy.txt" with p notitle

set title "Reconstructed data"
set ylabel "lag-h autocorrelation r_h"
set xlabel "h"
set xtics
plot [1:2048][-0.2:0.2] \
     "out1M.txt" with lines lw 2 notitle
