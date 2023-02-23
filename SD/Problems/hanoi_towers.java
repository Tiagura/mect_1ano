import java.util.Scanner;

class hanoi_towers {
    public static void main(String[] args) {
        Scanner input = new Scanner(System.in);
        System.out.print("Enter number of disks: ");
        int n_disks = input.nextInt();
        System.out.print("Enter number of towers: ");
        int n_towers = input.nextInt();

        //create towers
        tower[] towers = new tower[n_towers];
        for (int i = 0; i < n_towers; i++){
            towers[i] = new tower(n_disks, "Tower " + i);
        }

        //add disks to first tower A+n_disks-1 to A
        for (int i = 0; i < n_disks; i++){
            towers[0].add_disk((char)('A' + n_disks - 1 - i));
        }

        //print towers
        for (int i = 0; i < n_towers; i++){
            System.out.println(towers[i].toString());
        }

        //move disks
        resolve((char)('A' + n_disks - 1), towers[0], towers[n_towers - 1], towers[1]);

        //print towers
        for (int i = 0; i < n_towers; i++){
            System.out.println(towers[i].toString());
        }

    }

    private static void resolve(char disk, tower source, tower destination, tower aux){
        if (disk == 'A'){
            destination.add_disk(source.remove_disk());
            System.out.println("Moved disk " + disk + " from " + source.get_name() + " to " + destination.get_name());
        }
        else{
            resolve((char)(disk - 1), source, aux, destination);
            destination.add_disk(source.remove_disk());
            System.out.println("Moved disk " + disk + " from " + source.get_name() + " to " + destination.get_name());
            resolve((char)(disk - 1), aux, destination, source);
        }
    }

}

class tower {
    private int size, n_disks, top, next;
    private char[] disks;
    private String name;

    public tower(int size, String name){
        this.name = name;
        this.size = size;
        this.n_disks = 0;
        this.top = 0;
        this.next = 0;
        this.disks = new char[size];
        //fill disks with |
        for (int i = 0; i < size; i++){
            disks[i] = '-';
        }
    }

    public int add_disk(char disk){
        if (n_disks == size){
            System.out.println("Stack is full");
            return 0;
        }
        //check if disk is smaller than the last one
        if (n_disks < 0 && disks[top] > disk){
            System.out.println(this.toString());
            System.out.println("Disk is too big");
            return 0;
        }
        disks[next]=disk;
        top = next;
        next = (next + 1) % size;
        n_disks++;
        return 1;
    }

    public char remove_disk(){
        if (n_disks == 0){
            System.out.println("Stack is empty");
            return 0;
        }
        char disk = disks[top];
        next = top;
        top = (top - 1) % size;
        n_disks--;
        return disk;
    }

    public char get_top(){
        if (n_disks == 0){
            System.out.println("Stack is empty");
            return 0;
        }
        return disks[top];
    }

    public String get_name(){
        return name;
    }

    public String toString(){
        //get disks from top to bottom
        String disks = name+": ";
        for (int i = 0; i < n_disks; i++){
            disks += this.disks[(top - i) % size] + " ";
        }
        return disks;
    }
}