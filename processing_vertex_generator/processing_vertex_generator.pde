import java.io.FileWriter;
import java.io.IOException;

class CColor 
{
    float r, g, b;
    public CColor(float r, float g, float b)
    {
        this.r = r;
        this.g = g; 
        this.b = b;
    }
}

float width  = 500,
      height = 500;

ArrayList<PVector> vertices = new ArrayList<>();
ArrayList<CColor> vert_colors = new ArrayList<>();

ArrayList<CColor> colors = new ArrayList<>();
int curr_color = 0;

void settings()
{
    size((int)width, (int)height);
}

void setup()
{
    colors.add(new CColor(243, 228, 89));
    colors.add(new CColor(51, 25, 0));
    colors.add(new CColor(0, 0, 0));
}

void draw()
{
    background(100);
    CColor curr = colors.get(curr_color);
    fill(curr.r, curr.g, curr.b);
    circle(10, 10, 10);

    for (int i = 0; i < vertices.size(); i++)
    {
        PVector vert = vertices.get(i);
        CColor c = vert_colors.get(i);
        fill(c.r, c.g, c.b);
        circle(vert.x, vert.y, 5);
    }
}

void mouseClicked()
{
    if (mouseButton == RIGHT) curr_color = (curr_color + 1) % 3;
    else 
    {
        PVector mouse_vec = new PVector(mouseX, mouseY);
        for (int i = 0; i < vertices.size(); i++)
        {
            PVector vert = vertices.get(i);
            if (vert.dist(mouse_vec) < 5)
            {
                println("Adding same again");
                vertices.add(vert);
                vert_colors.add(colors.get(curr_color));
                return;
            }   
        }

        vert_colors.add(colors.get(curr_color));
        vertices.add(new PVector(mouseX, mouseY));
    }
}

void keyPressed()
{
    if (key == 's')
    {
        println("Saving Image!");
        try {
            FileWriter writer = new FileWriter("D:\\Projects\\CSCI-5607\\processing_vertex_generator\\picture.txt");
            
            for (int i = 0; i < vertices.size(); i++)
            {
                CColor c = vert_colors.get(i);
                PVector v = vertices.get(i);

                String s = "v ";
                s += String.format("%f %f %f %f %f %f\n",
                    (v.x - 250) / 250, -1*((v.y - 250) / 250), 0.0f, c.r/255, c.g/255, c.b/255
                );
                writer.write(s);
            }

            writer.flush();
            writer.close();
        } 
        catch (IOException e)
        {
            println("Shit");
        }
    }
}

