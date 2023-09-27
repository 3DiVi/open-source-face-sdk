package com.face_detector_demo;

import com.face_sdk.Service;
import com.face_sdk.Context;
import com.face_sdk.ProcessingBlock;

import java.io.File;
import java.io.IOException;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Color;
import java.awt.Rectangle;
import java.awt.BasicStroke;
import java.awt.image.BufferedImage;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.WindowConstants;
import javax.imageio.ImageIO;

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;


public class face_detector_demo {
	public static void main(String[] args) throws Exception {

		if (args.length < 2){
			throw new Exception("need <image_path> <sdk_path>");
		}

		String imagePath = args[0];
		String sdk_path = args[1];

		Service service = Service.createService(sdk_path);

		Context modelCtx = service.createContext();
		modelCtx.getOrInsertByKey("unit_type").setString("FACE_DETECTOR");

		// create processing block
		ProcessingBlock detector = service.createProcessingBlock(modelCtx);

		// prepare image
		BufferedImage image = ImageIO.read(new File(imagePath));
		int w = image.getWidth();
		int h = image.getHeight();

		int[] rgba_array = image.getRGB(0, 0, w, h, null, 0, w);
		byte[] blob = new byte[w * h * 3];

		for (int i = 0; i < w * h; i++){
			blob[i * 3] = (byte)((rgba_array[i] >>> 16) & 0xFF);
			blob[i * 3 + 1] = (byte)((rgba_array[i] >>> 8) & 0xFF);
			blob[i * 3 + 2] = (byte)((rgba_array[i] >>> 0) & 0xFF);
		}

		// create image context and put into ioData
		Context ioData = service.createContext();

		Context imgCtx = ioData.getOrInsertByKey("image");
		imgCtx.getOrInsertByKey("blob").setDataPtr((byte[])blob);
		imgCtx.getOrInsertByKey("dtype").setString("uint8_t");
		imgCtx.getOrInsertByKey("format").setString("NDARRAY");
		Context shapeCtx = imgCtx.getOrInsertByKey("shape");

		Context hCtx = service.createContext();
		Context wCtx = service.createContext();
		Context cCtx = service.createContext();

		hCtx.setLong(h);
		wCtx.setLong(w);
		cCtx.setLong(3);

		shapeCtx.pushBack(hCtx);
		shapeCtx.pushBack(wCtx);
		shapeCtx.pushBack(cCtx);

		///////////Detector////////////////
		detector.process(ioData);
		///////////////////////////////////

		JFrame frame = new JFrame();
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setSize(w, h);

		JPanel pane = new JPanel() {
			@Override
			protected void paintComponent(Graphics g) {
				super.paintComponent(g);
				Graphics2D g2 = (Graphics2D) g;
				g2.drawImage(image, 0, 0, null);
				g2.setColor(Color.RED);
				g2.setStroke(new BasicStroke(5));

				Context objects = ioData.getOrInsertByKey("objects");
				for (int i = 0; i < objects.size(); i++){
					Context bbox = objects.getByIndex(i).getByKey("bbox");
					int x = (int)(bbox.getByIndex(0).getDouble() * w);
					int y = (int)(bbox.getByIndex(1).getDouble() * h);
					int x2 = (int)(bbox.getByIndex(2).getDouble() * w);
					int y2 = (int)(bbox.getByIndex(3).getDouble() * h);
					g2.drawRect(x, y, x2 - x, y2 - y);
				}
			}
		};

		KeyAdapter listener = new KeyAdapter() {
			@Override public void keyPressed(KeyEvent e) {
				frame.dispose();
			}
		};

		frame.add(pane);
		frame.setVisible(true);
		frame.addKeyListener(listener);
	}
}