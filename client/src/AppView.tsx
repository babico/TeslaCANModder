import AppExperience from "./AppExperience";
import { BoardConnectionProvider } from "./state/BoardConnectionContext";
import { SafeAreaProvider } from "react-native-safe-area-context";

export default function AppView() {
  return (
    <SafeAreaProvider>
      <BoardConnectionProvider>
        <AppExperience />
      </BoardConnectionProvider>
    </SafeAreaProvider>
  );
}
