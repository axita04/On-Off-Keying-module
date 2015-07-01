load sim.txt;

x = sim(:,2);
y = sim(:,1);

plot(x, y, 'linewidth', 2);

title('Throughput');
xlabel('distance [m]');
ylabel('Throughput');
