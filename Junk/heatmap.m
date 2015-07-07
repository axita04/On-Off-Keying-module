% reads in the file text file created by ns3 simulation as a matrix
z = dlmread('illumination.txt')

% that scales the z-values and plots a heatmap
imagesc(z)

title('Illumination of a 5x5 room')
xlabel('x [0.25m]')
ylabel('y [0.25m]')

% choose a colormap of your liking
colormap('hot');
colorbar;
