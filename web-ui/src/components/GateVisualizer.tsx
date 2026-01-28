import { useVisualizerData } from '../hooks/useVisualizerData';

interface GateVisualizerProps {
  numSteps: number;
  pattern: number;
}

export function GateVisualizer({ numSteps, pattern }: GateVisualizerProps) {
  const data = useVisualizerData();

  // Generate steps array
  const steps = Array.from({ length: 16 }, (_, i) => {
    const isActive = i < numSteps;
    const isOn = (data.stepPattern >> (15 - i)) & 1;
    const isCurrent = i === data.currentStep;
    return { index: i, isActive, isOn, isCurrent };
  });

  return (
    <div className="gate-visualizer">
      {/* Step sequencer display */}
      <div className="step-grid">
        {steps.map(step => (
          <div
            key={step.index}
            className={`step ${step.isActive ? 'active' : 'inactive'} ${step.isOn ? 'on' : 'off'} ${step.isCurrent ? 'current' : ''}`}
          >
            <div className="step-indicator" style={{
              height: `${step.isCurrent && step.isOn ? data.gateLevel * 100 : (step.isOn ? 70 : 20)}%`
            }} />
            <span className="step-number">{step.index + 1}</span>
          </div>
        ))}
      </div>

      {/* Output level meter */}
      <div className="output-meter">
        <div className="meter-fill" style={{ width: `${data.outputLevel * 100}%` }} />
        <span className="meter-label">OUTPUT</span>
      </div>
    </div>
  );
}
