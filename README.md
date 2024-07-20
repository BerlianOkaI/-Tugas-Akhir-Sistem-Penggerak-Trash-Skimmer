# Tugas Akhirku - Sistem Penggerak Trash Skimmer

_Disusun oleh Berlian Oka Irvianto, 2024_

## Apa itu Trash Skimmer / What is Trash Skimmer?
Trash skimmer merupakan alat berupa kapal / perahu dengan suatu bak penampungan di dalamnya yang bertujuan untuk menampung sampah yang diambil Trash Skimmer di permukaan air. Dengan kata lain, Trash Skimmer merupakan solusi yang hendak dibuat untuk mengatasi masalah sampah yang ada di perairan, khususnya di permukaan air. Trash Skimmer dapat bergerak dengan menggunakan energi listrik. Energi listrik ini diperoleh melalui panel surya berkapasitas 300 WP atau diperoleh melalui baterai aki (*Lead Acid Battery*) dengan tegangan keluaran sebesar 12 V. Trash Skimmer digerakkan dengan menggunakan dua buah motor BLDC (*Brushless Direct Current*) yang lajunya dikendalikan oleh masing-masing ESC (*Electronic Speed Controller*). Selain itu, untuk mengangkut sampah dari perairan menuju bak penampungan, sebuah konveyor yang digerakkan oleh DC Gearbox Motor 12 V digunakan.


> Trash Skimmer is a device in the form of boat in which it has a waste storage tank which serves to store the surface water waste. In other word, Trash Skimmer is a one of many solution to solve the surface water waste problem. The Trash Skimmer can move using electrical energy. The energy is provided from solar panel with capacity of 300 WP and from a lead acid battery with 12 V output voltage. The Trash Skimmer is driven by two Brushless DC (BLDC) Motor where the speed of these BLDCs are controlled by an Electronic Speed Controller (ESC). In addition, a conveyor which is driven by a 12 V DC Gearbox Motor is used to take up a surface water waste into the waste storage tank.


## Sistem Pengirim dan Penerima pada Trash Skimmer / Transmitter and Receiver System of Trash Skimmer
Sistem Trash Skimmer terbagi menjadi dua bagian, yaitu sistem pengirim dan sistem penerima. Sistem pengirim merupakan rangkaian listrik yang berisikan tombol-tombol dan sebuah joystick yang berfungsi untuk mengirimkan perintah gerak menuju sistem penerima melalui sambungan nirkabel RF 433. Perintah-perintah yang dikirimkan tersebut meliputi perintah gerak (maju dan belok) dari Trash Skimmer dan perintah kontrol konveyor. Sementara itu, sistem penerima Trash Skimmer bertugas untuk menerima sinyal yang berisikan perintah yang dikirimkan sistem pengirim yang selanjutnya perintah tersebut digunakan untuk mengontrol kedua propeller dan konveyor.  
