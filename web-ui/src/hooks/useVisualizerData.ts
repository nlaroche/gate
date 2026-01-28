import { useState, useEffect, useCallback } from 'react';
import { isInJuceWebView, addEventListener, removeEventListener } from '../lib/juce-bridge';

export interface GateVisualizerData {
  currentStep: number;
  gateLevel: number;
  outputLevel: number;
  stepPattern: number;
}

const defaultData: GateVisualizerData = {
  currentStep: 0,
  gateLevel: 0,
  outputLevel: 0,
  stepPattern: 0xFFFF,
};

export function useVisualizerData(): GateVisualizerData {
  const [data, setData] = useState<GateVisualizerData>(defaultData);

  const handleVisualizerData = useCallback((eventData: any) => {
    if (eventData && typeof eventData === 'object') {
      setData({
        currentStep: eventData.currentStep ?? 0,
        gateLevel: eventData.gateLevel ?? 0,
        outputLevel: eventData.outputLevel ?? 0,
        stepPattern: eventData.stepPattern ?? 0xFFFF,
      });
    }
  }, []);

  useEffect(() => {
    if (!isInJuceWebView()) {
      // Demo mode animation
      let animationFrame: number;
      let step = 0;
      let time = 0;

      const animate = () => {
        time += 0.016;

        // Simulate step sequencer at ~120 BPM, 1/8 notes
        if (time >= 0.25) {
          time = 0;
          step = (step + 1) % 16;
        }

        const pattern = 0xEEEE; // Trance pattern
        const isOn = (pattern >> (15 - step)) & 1;

        setData({
          currentStep: step,
          gateLevel: isOn ? 0.8 + Math.random() * 0.2 : 0.1,
          outputLevel: isOn ? 0.6 + Math.random() * 0.2 : 0.1,
          stepPattern: pattern,
        });

        animationFrame = requestAnimationFrame(animate);
      };

      animate();
      return () => cancelAnimationFrame(animationFrame);
    }

    addEventListener('visualizerData', handleVisualizerData);
    return () => removeEventListener('visualizerData', handleVisualizerData);
  }, [handleVisualizerData]);

  return data;
}
