//based on http://graphstream-project.org/gs-talk/demos.html#/a-first-dynamic-graph
import org.graphstream.graph.Graph;
import org.graphstream.graph.implementations.SingleGraph;
import org.graphstream.stream.file.FileSource;
import org.graphstream.stream.file.FileSourceDGS;

import java.io.IOException;

/*  Jars Used:
        gs-core-1.3\gs-core-1.3.jar
        gs-core-1.3\gs-core-1.3-javadoc.jar
        gs-core-1.3\gs-core-1.3-sources.jar
        gs-ui-1.3\gs-ui-1.3.jar
        gs-ui-1.3\gs-ui-1.3-javadoc.jar
        gs-ui-1.3\gs-ui-1.3-sources.jar
    Note:
        dht.c, dht.dgs, dht.java must be located in the same directory
        When running dht.java, needed the absolute path to run dht.dgs
        on the school computer. May be different when you run it.
    Helpful Links:
        http://graphstream-project.org/doc/Advanced-Concepts/The-DGS-File-Format/
            How to write a .dgs file
        http://graphstream-project.org/doc/Advanced-Concepts/GraphStream-CSS-Reference/
            Shows you how to do cool stuff
*/
public class dht {
        
    public static void main(String args[]) throws IOException, InterruptedException {
             
        System.setProperty("org.graphstream.ui.renderer", "org.graphstream.ui.j2dviewer.J2DGraphRenderer");
        Graph graph = new SingleGraph("Demo2");
        graph.setAttribute("ui.antialias");
        graph.setAttribute("ui.quality");
        graph.addAttribute("ui.stylesheet", "graph {padding: 20px;}" +
                           "node {size: 22px; fill-mode: plain; fill-color: rgb(210, 210, 210); stroke-mode: plain; text-color: black;}" +
                               "node.online {fill-color: white; stroke-mode: none; text-color: black; text-background-mode: rounded-box; text-background-color: rgb(130, 252, 0); text-padding: 2;}" +
                               "node.visited {size: 30; fill-mode: gradient-vertical; fill-color: rgb(110, 160, 200), rgb(150, 200, 255);}" +
                               "node.key {size: 30; fill-mode: plain; fill-color: rgb(130, 252, 0);}" +
                               "node.invisible {fill-color: white; stroke-mode: none; text-color: black; text-size: 15;}" +
                            "edge {fill-mode: plain;}" +
                               "edge.path {fill-color: red;}" +
                               "edge.dash {fill-color:green;}");
                
        graph.display(false);
            
        FileSource source = new FileSourceDGS();
        source.addSink(graph);
            
        // depending on how you run this, might need the absolute path to get to dht.dgs
        //String abspath = System.getProperty("user.dir");    
        //System.out.println("Looking at file path: " + abspath + " for dht.dgs");
        
        //source.begin(abspath + "\\dht.dgs");
        source.begin("dht.dgs");
        
        while(source.nextStep()) {
                
            Thread.sleep(2000);
        }
                
        source.end();
    }
}