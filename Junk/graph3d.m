%MATLAB code to graph in 3d

fid=fopen('sim2.txt');
XYZ=fscanf(fid,'%f %f %f',[3 Inf]);
fclose(fid);

plot3(XYZ(1,:), XYZ(2,:), XYZ(3,:));

title('SNR in a 5x5x5 room');
xlabel('x [m]');
ylabel('y [y]');
zlabel('SNR');

disp('Minimum: ');
disp(min(XYZ(3,:)));

disp('Maximum: ');
disp(max(XYZ(3,:)));

%in the script used to output data points the following code should be added:
%     std::ofstream otherfile;
%     otherfile.open("sim2.txt");
%     // calculations and other code goes here

%     otherfile << distx << " " << disty << x.SNR()+y.SNR()+z.SNR() << std::endl;
%     otherfile.close();
